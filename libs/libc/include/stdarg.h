#ifndef _LIBC_STDARG_H
#define _LIBC_STDARG_H

#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

#define va_start(v, l) __builtin_va_start(v, l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v, l) __builtin_va_arg(v, l)

__END_DECLS

#endif // _LIBC_STDARG_H