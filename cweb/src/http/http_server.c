#include "http/http.h"
#include "http/http_paser_internal.h"
#include "http/http_request_internal.h"
#include "http/http_response_internal.h"

#include <string.h>
#include <stdlib.h>

#define ROUTE_HASH_SIZE 128

typedef struct RouteEntry {
    char* route;
    RouteHandler handler;
    struct RouteEntry* next;
} RouteEntry;

typedef struct RouteBucket {
    RouteEntry* head;
} RouteBucket;

static RouteBucket route_table[4][ROUTE_HASH_SIZE];

static uint32_t hash_route(const char* str) {
    uint32_t hash = 5381;
    while (*str) {
        hash = ((hash << 5) + hash) + (unsigned char)(*str);
        str++;
    }
    return hash % ROUTE_HASH_SIZE;
}

static void register_route(HttpMethod method, const char* route, RouteHandler handler) {
    if (!route || !handler) return;
    uint32_t h = hash_route(route);

    RouteEntry* entry = malloc(sizeof(RouteEntry));
    entry->route = strdup(route);
    entry->handler = handler;
    entry->next = route_table[method][h].head;
    route_table[method][h].head = entry;
}

void register_get_route(const char* route, RouteHandler handler)    { register_route(GET, route, handler); }
void register_post_route(const char* route, RouteHandler handler)   { register_route(POST, route, handler); }
void register_put_route(const char* route, RouteHandler handler)    { register_route(PUT, route, handler); }
void register_delete_route(const char* route, RouteHandler handler) { register_route(DEL, route, handler); }

void handle_client(NetSocket* s, NetSocket* client) {
    char buf[4096];
    int n = net_recv(client, buf, sizeof(buf)-1);
    if (n <= 0) return;
    buf[n] = '\0';

    HttpRequest* req = parse_http_request(buf, n);
    if (!req) return;

    HttpResponse* res = malloc(sizeof(HttpResponse));
    if (!res) { free_request(req); return; }
    memset(res, 0, sizeof(HttpResponse));

    // 查找 handler
    uint32_t h = hash_route(req->route);
    RouteEntry* e = route_table[req->method][h].head;
    while (e) {
        if (strcmp(e->route, req->route) == 0) {
            e->handler(req, res);
            break;
        }
        e = e->next;
    }

    // 如果没有匹配
    if (!e) {
        http_response_status_not_found(res);
        http_response_set_text(res, "Route not found");
    }

    // 生成并发送响应
    size_t len;
    char* resp_buf = build_http_response(res, &len);
    if (resp_buf) {
        net_send(client, resp_buf, len);
        free(resp_buf);
    }

    free_request(req);
    free_response(res);
}
