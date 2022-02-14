#ifndef _KERNEL_LIBKERN_PRINTF_H
#define _KERNEL_LIBKERN_PRINTF_H

#include <libkern/types.h>

typedef int (*printf_putch_callback)(char ch, char* buf_base, size_t* written, void* callback_params);

int vsnprintf(char* s, size_t n, const char* format, va_list arg);
int vsprintf(char* s, const char* format, va_list arg);
int snprintf(char* s, size_t n, const char* format, ...);
int sprintf(char* s, const char* format, ...);

int printf_engine(char* buf, const char* format, printf_putch_callback callback, void* callback_params, va_list arg);

#endif // _KERNEL_LIBKERN_PRINTF_H