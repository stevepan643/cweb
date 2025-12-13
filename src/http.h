#ifndef HTTP_H
#define HTTP_H

#include "platform.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef enum HttpMethod {
    GET, POST, PUT, DELETE
} HttpMethod;

typedef struct HttpRequest HttpRequest;

typedef struct HttpResponse HttpResponse;

typedef void (*RouteHandler)(const HttpRequest *req, HttpResponse *res);

void register_route(HttpMethod method, const char* route, RouteHandler handler);
void handle_client(NetSocket* s, NetSocket* client);

HttpMethod http_request_get_method(const HttpRequest *req);
const char* http_request_get_route(const HttpRequest *req);
const char* http_request_get_header(const HttpRequest *req, const char* key);
const char* http_request_get_body(const HttpRequest *req, size_t* length);

// void http_response_set_status(HttpResponse* res, int status, const char* status_text);
void http_response_status_ok(HttpResponse* res);
void http_response_status_not_found(HttpResponse* res);
void http_response_status_error(HttpResponse* res);
// void http_response_set_body(HttpResponse* res, const char* body, size_t length);
void http_response_set_text(HttpResponse* res, const char* text);
void http_response_set_json(HttpResponse* res, const char* json, ...);
void http_response_add_header(HttpResponse* res, const char* key, const char* value);
void http_response_set_file(HttpResponse* res, const char* filepath);

#endif