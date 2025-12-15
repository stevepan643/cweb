#include "http/http.h"
#include "http/http_internal.h"
#include "http/http_response_internal.h"
#include "utils/file/file.h"

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
    res->body = malloc(res->body_length + 1); // 留出 '\0'
    if (!res->body) return;

    memcpy(res->body, text, res->body_length);
    res->body[res->body_length] = '\0'; // 终止符

    http_response_add_header(res, "Content-Type", "text/plain");
}
void http_response_set_text_len(HttpResponse* res, const char* text, size_t len)
{
    if (!res || !text) return;

    if (res->body) free(res->body);

    res->body = malloc(len + 1);
    if (!res->body) return;

    memcpy(res->body, text, len);
    res->body[len] = '\0'; // 保证终止符
    res->body_length = len;

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
    res->body = malloc(len + 1); // 多分配一个字节给 '\0'
    if (!res->body) return;

    va_start(args, json);
    vsnprintf(res->body, len + 1, json, args); // 写入 '\0'
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

    const char* http_version = "HTTP/1.1";
    char header_buf[1024];
    int pos = 0;

    char* body_buf = res->body;
    size_t body_len = res->body_length;
    int body_buf_allocated = 0; // 是否需要 free

    // --------- 处理文件 ---------
    if (res->file_path) {
        char* file_buf = file_read_all(res->file_path, &body_len);
        if (!file_buf) {
            // 文件读取失败，返回 404
            res->status = 404;
            res->status_text = "Not Found";
            body_buf = (char*)"File not found"; // 静态字符串
            body_len = strlen(body_buf);
        } else {
            body_buf = file_buf;
            body_buf_allocated = 1;  // 需要释放
            // 自动添加 Content-Type
            http_response_add_header(res, "Content-Type", "text/html; charset=utf-8");
        }
    }

    // --------- 构建 HTTP 头 ---------
    pos += snprintf(header_buf + pos, sizeof(header_buf) - pos,
                    "%s %d %s\r\n",
                    http_version,
                    res->status,
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
                    body_len);

    // --------- 分配完整缓冲区 ---------
    size_t total_len = pos + body_len;
    char* buffer = malloc(total_len);
    if (!buffer) {
        if (body_buf_allocated) free(body_buf);
        return NULL;
    }

    memcpy(buffer, header_buf, pos);
    if (body_len > 0 && body_buf) {
        memcpy(buffer + pos, body_buf, body_len);
    }

    if (out_len) *out_len = total_len;

    // 释放 file_read_all 分配的内存
    if (body_buf_allocated) free(body_buf);

    return buffer;
}