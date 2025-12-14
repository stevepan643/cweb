#ifndef LOGGER_INTERNAL_H
#define LOGGER_INTERNAL_H

#include "utils/log/logger.h"
#include "utils/file/file.h"
#include "utils/platform/platform.h"

struct Logger{
    LogLevel level;
    int use_color;
    char file_name_pattern[256];
    size_t max_file_size;
    Mutex* lock;
};

#endif