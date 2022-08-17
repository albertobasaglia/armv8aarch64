#ifndef LOG_H
#define LOG_H

#include <stdarg.h>
#include <stdint.h>

void klog(const char* str);

void klogf(const char* format, ...);

int sprintf_internal(char* str, const char* format, va_list vars);

int sprintf(char* str, const char* format, ...);

void uint64_to_string(char* str, uint64_t num, int base);

#endif
