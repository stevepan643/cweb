#include "utils/file/file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* file_read_all(const char* path, size_t* out_len)
{
    if (!path) return NULL;

    FILE* f = fopen(path, "rb");
    if (!f) return NULL;

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }

    long sz = ftell(f);
    if (sz < 0) {
        fclose(f);
        return NULL;
    }

    rewind(f);

    char* buf = (char*)malloc(sz + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }

    size_t read_sz = fread(buf, 1, sz, f);
    fclose(f);

    if (read_sz != (size_t)sz) {
        free(buf);
        return NULL;
    }

    buf[sz] = '\0';

    if (out_len) *out_len = sz;
    return buf;
}

int file_write_all(const char* path, const char* buf, size_t len)
{
    if (!path || !buf) return -1;

    FILE* f = fopen(path, "wb");
    if (!f) return -1;

    size_t written = fwrite(buf, 1, len, f);
    fclose(f);

    if (written != len) return -1;
    return 0;
}
