#ifndef _KERNEL_LIBKERN_LOG_H
#define _KERNEL_LIBKERN_LOG_H

#include <libkern/types.h>

void logger_setup();
void log(const char* format, ...);
void log_warn(const char* format, ...);
void log_error(const char* format, ...);
void log_char(char c);
void log_not_formatted(const char* format, ...);

#endif // _KERNEL_LIBKERN_LOG_H