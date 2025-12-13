#ifndef HTTP_RESPONSE_INTERNAL_H
#define HTTP_RESPONSE_INTERNAL_H

#include "http/http_internal.h"
#include "http/http_response.h"

struct HttpResponse {
    int status;
    const char *status_text;
    char *body;
    size_t body_length;
    HeaderTable headers;
    char* file_path;
};

char* build_http_response(HttpResponse* res, size_t* out_len);

#endif