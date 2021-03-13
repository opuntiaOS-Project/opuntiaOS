#ifndef _KERNEL_LIBKERN_STDARG_H
#define _KERNEL_LIBKERN_STDARG_H

#define va_start(v, l) __builtin_va_start(v, l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v, l) __builtin_va_arg(v, l)

#endif // _KERNEL_LIBKERN_STDARG_H