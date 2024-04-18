#ifndef __LOG_H__
#define __LOG_H__
#include <stdint.h>
#include <stdarg.h>

#include "main.h"

#define LOG_NONE 0
#define LOG_ERROR 1
#define LOG_WARN 2
#define LOG_INFO 3
#define LOG_ALL 4 
#define LOG_LEVEL LOG_ALL

#define log_printf(level, tag, fmt, ...) do { \
        printf("%s (%u) %s: " fmt "", level >= LOG_ALL ? "D" : \
                                     level >= LOG_INFO ? "I" : \
                                     level >= LOG_WARN ? "W" : \
                                     level >= LOG_ERROR ? "E" : "?", \
                                     millis(), \
                                     tag, ##__VA_ARGS__); \
    } while(0) 

#if LOG_LEVEL >= LOG_ERROR
#define LOG_E(tag, fmt, ...) log_printf(LOG_ERROR, tag, fmt, ##__VA_ARGS__)
#else
#define LOG_E(tag, fmt, ...)
#endif

#if LOG_LEVEL >= LOG_WARN
#define LOG_W(tag, fmt, ...) log_printf(LOG_WARN, tag, fmt, ##__VA_ARGS__)
#else
#define LOG_W(tag, fmt, ...)
#endif

#if LOG_LEVEL >= LOG_INFO
#define LOG_I(tag, fmt, ...) log_printf(LOG_INFO, tag, fmt, ##__VA_ARGS__)
#else
#define LOG_I(tag, fmt, ...)
#endif

#if LOG_LEVEL >= LOG_ALL
#define LOG_D(tag, fmt, ...) log_printf(LOG_ALL, tag, fmt, ##__VA_ARGS__)
#else
#define LOG_D(tag, fmt, ...)
#endif

#endif // !
