#include "http.h"
#include "platform.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define MAX_HEADER_SIZE 32

typedef struct HttpHeader {
    char header[64];
    char content[256];
} HttpHeader;

struct HttpRequest {
    HttpMethod method;
    char route[256];
    char version[64];
    HttpHeader headers[MAX_HEADER_SIZE];
    size_t header_count;
    char* content;
    size_t content_length;
};

struct HttpResponse {
    int status;
    const char *status_text;
    char *body;
    size_t body_length;
    HttpHeader headers[MAX_HEADER_SIZE];
    size_t header_count;
    char* file_path;
};

// ----------------- 内部函数 -----------------
static HttpMethod parse_method(const char* s) {
    if (strcmp(s, "GET") == 0) return GET;
    if (strcmp(s, "POST") == 0) return POST;
    if (strcmp(s, "PUT") == 0) return PUT;
    if (strcmp(s, "DELETE") == 0) return DELETE;
    return GET;
}

static char* build_http_response(HttpResponse* res, size_t* out_len) {
    if (!res) return NULL;

    // --------- 处理文件 ---------
    if (res->file_path) {
        FILE* f = fopen(res->file_path, "rb");
        if (f) {
            fseek(f, 0, SEEK_END);
            size_t sz = ftell(f);
            fseek(f, 0, SEEK_SET);

            if (res->body) free(res->body);
            res->body = malloc(sz);
            if (res->body) {
                fread(res->body, 1, sz, f);
                res->body_length = sz;
            } else {
                res->body_length = 0;
            }
            fclose(f);

            // 简单根据后缀设置 Content-Type
            http_response_add_header(res, "Content-Type", "text/html");
        } else {
            http_response_status_not_found(res);
            http_response_set_text(res, "File not found");
        }
    }

    const char* http_version = "HTTP/1.1";
    char header_buf[1024];
    int pos = 0;

    // 状态行
    pos += snprintf(header_buf + pos, sizeof(header_buf) - pos,
        "%s %d %s\r\n", http_version, res->status,
        res->status_text ? res->status_text : "");

    // 自定义 Header
    for (size_t i = 0; i < res->header_count; i++) {
        pos += snprintf(header_buf + pos, sizeof(header_buf) - pos,
            "%s: %s\r\n",
            res->headers[i].header,
            res->headers[i].content);
    }

    // Content-Length 和 Connection
    pos += snprintf(header_buf + pos, sizeof(header_buf) - pos,
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n",
        res->body_length);

    // 分配完整缓冲区
    size_t total_len = pos + res->body_length;
    char* buffer = malloc(total_len);
    if (!buffer) return NULL;

    memcpy(buffer, header_buf, pos);
    if (res->body && res->body_length > 0) {
        memcpy(buffer + pos, res->body, res->body_length);
    }

    if (out_len) *out_len = total_len;
    return buffer;
}

// ----------------- HTTP 请求解析 -----------------
HttpRequest* parse_http_request(const char* raw, size_t len) {
    if (!raw || len == 0) return NULL;

    HttpRequest* req = malloc(sizeof(HttpRequest));
    if (!req) return NULL;
    memset(req, 0, sizeof(HttpRequest));

    const char* line = raw;
    const char* next = strstr(line, "\r\n");
    if (!next) {
        free(req);
        return NULL;
    }

    // 解析请求行
    char request_line[512];
    size_t rl_len = next - line;
    if (rl_len >= sizeof(request_line)) rl_len = sizeof(request_line) - 1;
    strncpy(request_line, line, rl_len);
    request_line[rl_len] = '\0';

    char method[16], route[256], version[16];
    sscanf(request_line, "%15s %255s %15s", method, route, version);
    req->method = parse_method(method);
    strncpy(req->route, route, sizeof(req->route) - 1);
    strncpy(req->version, version, sizeof(req->version) - 1);

    line = next + 2;

    // 解析 headers
    while (line < raw + len) {
        next = strstr(line, "\r\n");
        if (!next) break;
        if (next == line) { // 空行，headers结束
            line += 2;
            break;
        }

        char header_line[512];
        size_t hl_len = next - line;
        if (hl_len >= sizeof(header_line)) hl_len = sizeof(header_line) - 1;
        strncpy(header_line, line, hl_len);
        header_line[hl_len] = '\0';

        char* sep = strstr(header_line, ":");
        if (sep && req->header_count < MAX_HEADER_SIZE) {
            *sep = '\0';
            sep++;
            while (*sep == ' ') sep++;
            strncpy(req->headers[req->header_count].header, header_line, 63);
            strncpy(req->headers[req->header_count].content, sep, 255);
            req->header_count++;
        }

        line = next + 2;
    }

    // 解析 body
    size_t body_len = raw + len - line;
    if (body_len > 0) {
        req->content = malloc(body_len);
        if (req->content) {
            memcpy(req->content, line, body_len);
            req->content_length = body_len;
        }
    }

    return req;
}

// ----------------- 内存释放 -----------------
void free_request(HttpRequest* req) {
    if (!req) return;
    if (req->content) free(req->content);
    free(req);
}

void free_response(HttpResponse *res) {
    if (!res) return;
    if (res->body) free(res->body);
    free(res);
}

// ----------------- 路由表 -----------------
typedef struct RouteEntry {
    HttpMethod method;
    char route[256];
    RouteHandler handler;
} RouteEntry;

#define MAX_ROUTES 32
static RouteEntry routes[MAX_ROUTES];
static size_t route_count = 0;

void register_route(HttpMethod method, const char* route, RouteHandler handler) {
    if (route_count >= MAX_ROUTES) return;
    routes[route_count].method = method;
    strncpy(routes[route_count].route, route, sizeof(routes[route_count].route) - 1);
    routes[route_count].handler = handler;
    route_count++;
}

// ----------------- 客户端处理 -----------------
void handle_client(NetSocket* s, NetSocket* client) {
    char buf[4096];
    int n = net_recv(client, buf, sizeof(buf)-1);
    if (n <= 0) return;
    buf[n] = '\0';

    HttpRequest* req = parse_http_request(buf, n);
    if (!req) return;

    HttpResponse* res = malloc(sizeof(HttpResponse));
    if (!res) {
        free_request(req);
        return;
    }
    memset(res, 0, sizeof(HttpResponse));

    // 查找路由并调用 handler
    for (size_t i = 0; i < route_count; i++) {
        if (req->method == routes[i].method && strcmp(req->route, routes[i].route) == 0) {
            routes[i].handler(req, res);
            break;
        }
    }

    size_t len;
    char* resp_buf = build_http_response(res, &len);
    if (resp_buf) {
        net_send(client, resp_buf, len);
        free(resp_buf);
    }

    free_request(req);
    free_response(res);
}


HttpMethod http_request_get_method(const HttpRequest *req) {
    return req ? req->method : GET;
}

const char* http_request_get_route(const HttpRequest *req) {
    return req ? req->route : NULL;
}

const char* http_request_get_header(const HttpRequest *req, const char* key) {
    if (!req || !key) return NULL;
    for (size_t i = 0; i < req->header_count; i++) {
        if (strcmp(req->headers[i].header, key) == 0)
            return req->headers[i].content;
    }
    return NULL;
}

const char* http_request_get_body(const HttpRequest *req, size_t* length) {
    if (!req) return NULL;
    if (length) *length = req->content_length;
    return req->content;
}

void http_response_add_header(HttpResponse* res, const char* key, const char* value)
{
    if (!res || !key || !value || res->header_count >= MAX_HEADER_SIZE) return;

    strncpy(res->headers[res->header_count].header, key, 63);
    strncpy(res->headers[res->header_count].content, value, 255);
    res->header_count++;
}
void http_response_status_ok(HttpResponse* res)
{
    if (!res) return;
    res->status = 200;
    res->status_text = "OK";
}
void http_response_status_not_found(HttpResponse* res)
{
    if (!res) return;
    res->status = 404;
    res->status_text = "Not Found";
}
void http_response_status_error(HttpResponse* res)
{
    if (!res) return;
    res->status = 500;
    res->status_text = "Internal Server Error";
}
void http_response_set_text(HttpResponse* res, const char* text)
{
    if (!res || !text) return;

    if (res->body) free(res->body);

    res->body_length = strlen(text);
    res->body = malloc(res->body_length);
    memcpy(res->body, text, res->body_length);

    http_response_add_header(res, "Content-Type", "text/plain");
}
void http_response_set_json(HttpResponse* res, const char* json, ...)
{
    if (!res || !json) return;

    va_list args;
    va_start(args, json);
    int len = vsnprintf(NULL, 0, json, args);
    va_end(args);

    if (res->body) free(res->body);
    res->body = malloc(len);
    if (!res->body) return;

    va_start(args, json);
    vsnprintf(res->body, len + 1, json, args);
    va_end(args);
    res->body_length = len;

    http_response_add_header(res, "Content-Type", "application/json");
}
void http_response_set_file(HttpResponse* res, const char* filepath)
{
    if (!res || !filepath) return;

    res->file_path = strdup(filepath);
}
