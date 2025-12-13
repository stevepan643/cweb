#include "http/http.h"
#include "http/http_internal.h"
#include "http/http_response_internal.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

void http_response_add_header(HttpResponse* res, const char* key, const char* value)
{
    if (!res || !key || !value || res->headers.count >= MAX_HEADER_SIZE) return;

    strncpy(res->headers.items[res->headers.count].key, key, 63);
    strncpy(res->headers.items[res->headers.count].value, value, 255);
    res->headers.count++;
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

void free_response(HttpResponse* res) {
    if (!res) return;

    if (res->body) {
        free(res->body);
        res->body = NULL;
        res->body_length = 0;
    }

    if (res->file_path) {
        free(res->file_path);
        res->file_path = NULL;
    }

    free(res);
}
char* build_http_response(HttpResponse* res, size_t* out_len) {
    if (!res) return NULL;

    // --------- 处理文件 ---------
    if (res->file_path) {
        // TODO
    }

    const char* http_version = "HTTP/1.1";
    char header_buf[1024];
    int pos = 0;

    // 状态行
    pos += snprintf(header_buf + pos, sizeof(header_buf) - pos,
        "%s %d %s\r\n", http_version, res->status,
        res->status_text ? res->status_text : "");

    // 自定义 Header
    for (size_t i = 0; i < res->headers.count; i++) {
        pos += snprintf(header_buf + pos, sizeof(header_buf) - pos,
            "%s: %s\r\n",
            res->headers.items[i].key,
            res->headers.items[i].value);
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