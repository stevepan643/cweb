#include "http/http.h"
#include "http/http_internal.h"
#include "http/http_request_internal.h"

#include <string.h>
#include <stdlib.h>

HttpMethod http_request_get_method(const HttpRequest *req) {
    return req ? req->method : GET;
}

const char* http_request_get_route(const HttpRequest *req) {
    return req ? req->route : NULL;
}

const char* http_request_get_header(const HttpRequest *req, const char* key) {
    if (!req || !key) return NULL;
    for (size_t i = 0; i < req->headers.count; i++) {
        if (strcmp(req->headers.items[i].key, key) == 0)
            return req->headers.items[i].value;
    }
    return NULL;
}

const char* http_request_get_body(const HttpRequest *req, size_t* length) {
    if (!req) return NULL;
    if (length) *length = req->content_length;
    return req->content;
}

void free_request(HttpRequest* req) {
    if (!req) return;
    if (req->content) free(req->content);
    free(req);
}