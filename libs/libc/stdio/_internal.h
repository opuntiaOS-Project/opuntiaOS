#ifndef _LIBC_STDIO__INTERNAL_H
#define _LIBC_STDIO__INTERNAL_H

#include <stdbool.h>
#include <stddef.h>
#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

typedef int (*_lookupch_callback)(void* callback_params);
typedef int (*_getch_callback)(void* callback_params);
typedef int (*_putch_callback)(char ch, char* buf_base, size_t* written, void* callback_params);

__END_DECLS

#endif // _LIBC_STDIO__INTERNAL_H
