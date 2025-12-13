#include "http/http_paser_internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static HttpMethod parse_method(const char* s) {
    if (!s) return GET;
    if (strcmp(s,"GET")==0) return GET;
    if (strcmp(s,"POST")==0) return POST;
    if (strcmp(s,"PUT")==0) return PUT;
    if (strcmp(s,"DELETE")==0) return DEL;
    return GET;
}

HttpRequest* parse_http_request(const char* raw, size_t len) {
    if (!raw || len == 0) return NULL;

    HttpRequest* req = malloc(sizeof(HttpRequest));
    if (!req) return NULL;
    memset(req, 0, sizeof(HttpRequest));

    const char* line = raw;
    const char* next = strstr(line, "\r\n");
    if (!next) { free(req); return NULL; }

    // 解析请求行
    char request_line[512];
    size_t rl_len = next - line;
    if (rl_len >= sizeof(request_line)) rl_len = sizeof(request_line) - 1;
    strncpy(request_line, line, rl_len);
    request_line[rl_len] = '\0';

    char method[16], route[256], version[16];
    sscanf(request_line, "%15s %255s %15s", method, route, version);

    req->method = parse_method(method);
    strncpy(req->route, route, sizeof(req->route)-1);
    strncpy(req->version, version, sizeof(req->version)-1);

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
        if (hl_len >= sizeof(header_line)) hl_len = sizeof(header_line)-1;
        strncpy(header_line, line, hl_len);
        header_line[hl_len] = '\0';

        char* sep = strstr(header_line, ":");
        if (sep && req->headers.count < MAX_HEADER_SIZE) {
            *sep = '\0';
            sep++;
            while (*sep == ' ') sep++;
            strncpy(req->headers.items[req->headers.count].key, header_line, 63);
            strncpy(req->headers.items[req->headers.count].value, sep, 255);
            req->headers.count++;
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