#ifndef _LIBC_SYS_IOCTL_H
#define _LIBC_SYS_IOCTL_H

#include <bits/sys/ioctls.h>
#include <stddef.h>
#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

int ioctl(int fd, uintptr_t cmd, uintptr_t arg);

__END_DECLS

#endif // _LIBC_SYS_IOCTL_H