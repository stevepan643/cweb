#include "utils/file/file.h"
#include "utils/platform/platform.h"
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

int mkdir_p(const char* path)
{
    if (!path || !*path) return -1;

    char tmp[512];
    size_t len;

    strncpy(tmp, path, sizeof(tmp));
    tmp[sizeof(tmp) - 1] = '\0';

    len = strlen(tmp);
    if (len == 0) return -1;

    // 去掉末尾分隔符
    if (tmp[len - 1] == '/' || tmp[len - 1] == '\\') {
        tmp[len - 1] = '\0';
    }

    for (char* p = tmp + 1; *p; p++) {
        if (*p == '/' || *p == '\\') {
            char c = *p;
            *p = '\0';

            if (mkdir(tmp) != 0) {
                return -1;
            }

            *p = c;
        }
    }

    // 创建最后一级
    if (mkdir(tmp) != 0) {
        return -1;
    }

    return 0;
}

int file_write_all(const char* path, const char* buf, size_t len)
{
    if (!path || !buf) return -1;

    char dir[512];
    strncpy(dir, path, sizeof(dir));
    dir[sizeof(dir) - 1] = '\0';

    char* slash1 = strrchr(dir, '/');
    char* slash2 = strrchr(dir, '\\');
    char* slash = slash1 > slash2 ? slash1 : slash2;

    if (slash) {
        *slash = '\0';
        if (mkdir_p(dir) != 0) {
            return -1;
        }
    }

    FILE* f = fopen(path, "ab");
    if (!f) return -1;

    size_t written = fwrite(buf, 1, len, f);
    fclose(f);

    return (written == len) ? 0 : -1;
}
