#ifndef _KERNEL_LIBKERN_LOG_H
#define _KERNEL_LIBKERN_LOG_H

#include <libkern/printf.h>
#include <libkern/types.h>

void logger_setup();

int log(const char* format, ...);
int log_warn(const char* format, ...);
int log_error(const char* format, ...);
int log_not_formatted(const char* format, ...);

#endif // _KERNEL_LIBKERN_LOG_H