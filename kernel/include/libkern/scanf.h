#ifndef _KERNEL_LIBKERN_SCANF_H
#define _KERNEL_LIBKERN_SCANF_H

#include <libkern/types.h>

#define EOF (-1)

typedef int (*scanf_lookupch_callback)(void* callback_params);
typedef int (*scanf_getch_callback)(void* callback_params);
typedef int (*scanf_putch_callback)(char ch, char* buf_base, size_t* written, void* callback_params);

int vsscanf(const char* buf, const char* fmt, va_list arg);
int sscanf(const char* buf, const char* fmt, ...);

int scanf_engine(const char* format, scanf_lookupch_callback lookupch, scanf_getch_callback getch, void* callback_params, va_list arg);

#endif // _KERNEL_LIBKERN_SCANF_H