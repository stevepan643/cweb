#ifndef HTTP_REQUEST_INTERNAL_H
#define HTTP_REQUEST_INTERNAL_H

#include "http/http_internal.h"
#include "http/http_request.h"

struct HttpRequest {
    HttpMethod method;
    char route[256];
    char version[64];
    HeaderTable headers;
    char* content;
    size_t content_length;
    time_t request_time;
};

#endif