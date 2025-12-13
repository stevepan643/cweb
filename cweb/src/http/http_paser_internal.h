#ifndef HTTP_PASER_INTERNAL_H
#define HTTP_PASER_INTERNAL_H

#include "http/http_request_internal.h"

HttpRequest* parse_http_request(const char* raw, size_t len);

#endif