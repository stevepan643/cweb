#include "http/http.h"
#include "http/http_paser_internal.h"
#include "http/http_request_internal.h"
#include "http/http_response_internal.h"

#include "utils/log/logger.h"

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
    LOG_TRACE("Waiting to receive data from client...");
    int n = net_recv(client, buf, sizeof(buf)-1);
    if (n <= 0) {
        LOG_WARN("Client disconnected or recv error: n=%d", n);
        return;
    }
    buf[n] = '\0';
    LOG_DEBUG("Received %d bytes from client", n);

    HttpRequest* req = parse_http_request(buf, n); // 假設 parse 也可以拿 client 填充 IP
    if (!req) {
        LOG_WARN("Failed to parse HTTP request");
        return;
    }

    const char* ip = net_get_ip(client);
    const uint16_t port = net_get_port(client);
    LOG_INFO("Request from: %s:%d -> %s %s", ip, port, 
                                                req->method == GET ? "GET" :
                                                req->method == POST ? "POST" : 
                                                req->method == PUT ? "PUT" : "DELETE",
                                                req->route);

    HttpResponse* res = malloc(sizeof(HttpResponse));
    if (!res) {
        LOG_ERROR("Failed to allocate HttpResponse");
        free_request(req);
        return;
    }
    memset(res, 0, sizeof(HttpResponse));

    // 查找 handler
    LOG_TRACE("Looking up handler for route: %s", req->route);
    uint32_t h = hash_route(req->route);
    RouteEntry* e = route_table[req->method][h].head;
    while (e) {
        if (strcmp(e->route, req->route) == 0) {
            LOG_DEBUG("Handler found for route: %s", req->route);
            e->handler(req, res);
            break;
        }
        e = e->next;
    }

    if (!e) {
        LOG_WARN("No handler matched for route: %s", req->route);
        http_response_status_not_found(res);
        http_response_set_text(res, "Route not found");
    }

    // 生成并发送响应
    LOG_TRACE("Building HTTP response...");
    size_t len;
    char* resp_buf = build_http_response(res, &len);
    if (resp_buf) {
        LOG_DEBUG("Sending response, %zu bytes", len);
        net_send(client, resp_buf, len);
        free(resp_buf);
        LOG_TRACE("Response sent successfully");
    } else {
        LOG_ERROR("Failed to build HTTP response");
    }

    free_request(req);
    free_response(res);
    LOG_TRACE("Finished handling client %s:%d", ip, port);
}

void* handle_client_task(void* arg) {
    ClientTaskArg* t_arg = (ClientTaskArg*)arg;
    NetSocket* s = t_arg->s;
    NetSocket* client = t_arg->client;

    free(t_arg); // 包装参数的内存可以释放

    char buf[4096];
    LOG_TRACE("Waiting to receive data from client...");
    int n = net_recv(client, buf, sizeof(buf)-1);
    if (n <= 0) {
        LOG_WARN("Client disconnected or recv error: n=%d", n);
        return NULL;
    }
    buf[n] = '\0';
    LOG_DEBUG("Received %d bytes from client", n);

    HttpRequest* req = parse_http_request(buf, n);
    if (!req) {
        LOG_WARN("Failed to parse HTTP request");
        return NULL;
    }

    const char* ip = net_get_ip(client);
    const uint16_t port = net_get_port(client);
    LOG_INFO("Request from: %s:%d -> %s %s", ip, port, 
             req->method == GET ? "GET" :
             req->method == POST ? "POST" : 
             req->method == PUT ? "PUT" : "DELETE",
             req->route);

    HttpResponse* res = malloc(sizeof(HttpResponse));
    if (!res) {
        LOG_ERROR("Failed to allocate HttpResponse");
        free_request(req);
        return NULL;
    }
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

    if (!e) {
        http_response_status_not_found(res);
        http_response_set_text(res, "Route not found");
    }

    size_t len;
    char* resp_buf = build_http_response(res, &len);
    if (resp_buf) {
        net_send(client, resp_buf, len);
        free(resp_buf);
    }

    free_request(req);
    free_response(res);

    net_close(client);

    LOG_TRACE("Finished handling client %s:%d", ip, port);
    return NULL;
}