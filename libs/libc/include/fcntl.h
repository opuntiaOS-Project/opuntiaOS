#ifndef _LIBC_FCNTL_H
#define _LIBC_FCNTL_H

#include <bits/fcntl.h>
#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

int open(const char* pathname, int flags, ...);
int creat(const char* path, mode_t mode);

__END_DECLS

#endif // _LIBC_FCNTL_H