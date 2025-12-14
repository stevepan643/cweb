#include "utils/log/logger_internal.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>

static Logger g_logger;

static const char* level_to_string(LogLevel level) {
    switch(level) {
        case LOG_TRACE: return "TRACE";
        case LOG_DEBUG: return "DEBUG";
        case LOG_INFO:  return "INFO";
        case LOG_WARN:  return "WARN";
        case LOG_ERROR: return "ERROR";
        case LOG_FATAL: return "FATAL";
    }
    return "UNKNOWN";
}

static const char* level_to_color(LogLevel level) {
    switch(level) {
        case LOG_TRACE: return "\x1b[37m";
        case LOG_DEBUG: return "\x1b[36m";
        case LOG_INFO:  return "\x1b[32m";
        case LOG_WARN:  return "\x1b[33m";
        case LOG_ERROR: return "\x1b[31m";
        case LOG_FATAL: return "\x1b[35m";
    }
    return "\x1b[0m";
}

static void get_file_name(char* buf, size_t size) {
    time_t t = time(NULL);
    struct tm tm;
    localtime_safe(&t, &tm);
    strftime(buf, size, g_logger.file_name_pattern, &tm);
}

void log_init(LogLevel level, int use_color, const char* file_pattern) {
    g_logger.level = level;
    g_logger.use_color = use_color;
    strncpy(g_logger.file_name_pattern, file_pattern, sizeof(g_logger.file_name_pattern)-1);
    g_logger.file_name_pattern[sizeof(g_logger.file_name_pattern)-1] = '\0';
    g_logger.max_file_size = 0; // 默认无限大
    g_logger.lock = mutex_create();
}

void log_log(LogLevel level, const char* file, int line, const char* fmt, ...) {
    if(level < g_logger.level) return;

    char timebuf[64];
    time_t t = time(NULL);
    struct tm tm;
    localtime_safe(&t, &tm);
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &tm);

    char msg[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);

    char final_msg[2048];
    snprintf(final_msg, sizeof(final_msg), "[%s] [%s] %s:%d: %s\n",
             timebuf, level_to_string(level), file, line, msg);

    mutex_lock(g_logger.lock);

    // 终端输出
    if(g_logger.use_color) {
        printf("%s%s\x1b[0m", level_to_color(level), final_msg);
    } else {
        printf("%s", final_msg);
    }
    fflush(stdout);

    // 文件输出
    char fname[256];
    get_file_name(fname, sizeof(fname));
    file_write_all(fname, final_msg, strlen(final_msg));

    mutex_unlock(g_logger.lock);
}

void log_add_file(const char* path, size_t max_size, int max_files) {
    mutex_lock(g_logger.lock);
    strncpy(g_logger.file_name_pattern, path, sizeof(g_logger.file_name_pattern)-1);
    g_logger.file_name_pattern[sizeof(g_logger.file_name_pattern)-1] = '\0';
    g_logger.max_file_size = max_size;
    mutex_unlock(g_logger.lock);
}

void log_add_stdout() {
}

void log_set_color(int enable) {
    mutex_lock(g_logger.lock);
    g_logger.use_color = enable;
    mutex_unlock(g_logger.lock);
}

void log_shutdown() {
    mutex_lock(g_logger.lock);
    mutex_free(g_logger.lock);
}
