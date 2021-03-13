#ifndef _KERNEL_LIBKERN_LOG_H
#define _KERNEL_LIBKERN_LOG_H

#include <libkern/types.h>

void logger_setup();

int vsnprintf(char* s, size_t n, const char* format, va_list arg);
int vsprintf(char* s, const char* format, va_list arg);
int snprintf(char* s, size_t n, const char* format, ...);
int sprintf(char* s, const char* format, ...);

int log(const char* format, ...);
int log_warn(const char* format, ...);
int log_error(const char* format, ...);
int log_not_formatted(const char* format, ...);

#endif // _KERNEL_LIBKERN_LOG_H