#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include "http/http.h"
#include <stddef.h>

HttpMethod http_request_get_method(const HttpRequest *req);
const char* http_request_get_route(const HttpRequest *req);
const char* http_request_get_header(const HttpRequest *req, const char* key);
const char* http_request_get_body(const HttpRequest *req, size_t* length);

void free_request(HttpRequest* req);

#endif
