#ifndef _LIBC_SYS_MMAN_H
#define _LIBC_SYS_MMAN_H

#include <bits/sys/mman.h>
#include <stddef.h>
#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

void* mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset);
int munmap(void* addr, size_t length);

__END_DECLS

#endif // _LIBC_SYS_MMAN_H