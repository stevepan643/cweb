#ifndef HTTP_H
#define HTTP_H

#include "utils/platform/platform.h"

typedef struct HttpResponse HttpResponse;
typedef struct HttpRequest HttpRequest;

typedef enum HttpMethod {
    GET, POST, PUT, DEL
} HttpMethod;

typedef void (*RouteHandler)(const HttpRequest*, HttpResponse*);

void register_get_route(const char* route, RouteHandler handler);
void register_put_route(const char* route, RouteHandler handler);
void register_post_route(const char* route, RouteHandler handler);
void register_delete_route(const char* route, RouteHandler handler);
void handle_client(NetSocket* s, NetSocket* client);

#endif
