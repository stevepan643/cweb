#ifndef LOGGER_H
#define LOGGER_H

#include <stdint.h>
#include <stddef.h>

typedef enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL } LogLevel;

typedef struct Logger Logger;

void log_init(LogLevel level, int use_color, const char* file_pattern);
void log_log(LogLevel level, const char* file, int line, const char* fmt, ...);
void log_add_file(const char* path, size_t max_size, int max_files);
void log_add_stdout();
void log_set_color(int enable);
void log_shutdown();

#define LOG_TRACE(fmt, ...) log_log(LOG_TRACE, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) log_log(LOG_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  log_log(LOG_INFO,  __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  log_log(LOG_WARN,  __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) log_log(LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_FATAL(fmt, ...) log_log(LOG_FATAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#endif