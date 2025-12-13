#ifndef FILE_H
#define FILE_H

#include <stddef.h>

char* file_read_all(const char* path, size_t* out_len);
int   file_write_all(const char* path, const char* buf, size_t len);

#endif