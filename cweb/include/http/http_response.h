#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include "http/http.h"

void http_response_status_ok(HttpResponse* res);
void http_response_status_not_found(HttpResponse* res);
void http_response_status_error(HttpResponse* res);

void http_response_set_text(HttpResponse* res, const char* text);
void http_response_set_json(HttpResponse* res, const char* json, ...);
void http_response_add_header(HttpResponse* res, const char* key, const char* value);
void http_response_set_file(HttpResponse* res, const char* filepath);

void free_response(HttpResponse* res);

#endif
