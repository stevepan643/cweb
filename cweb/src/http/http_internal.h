#ifndef HTTP_INTERNAL_H
#define HTTP_INTERNAL_H

#include <stdint.h>
#include <stddef.h>

#define MAX_HEADER_SIZE 32

typedef struct HttpHeader {
    char key[64];
    char value[256];
} HttpHeader;

typedef struct {
    HttpHeader items[MAX_HEADER_SIZE];
    size_t count;
} HeaderTable;

#endif