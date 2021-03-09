#ifndef _KERNEL_LIBKERN_BITS_SELECT_H
#define _KERNEL_LIBKERN_BITS_SELECT_H

#include <libkern/types.h>

#define FD_SETSIZE 8

struct fd_set {
    uint8_t fds[FD_SETSIZE / 8];
};
typedef struct fd_set fd_set_t;

#define FD_SET(fd, fd_set_ptr) ((fd_set_ptr)->fds[fd / 8] |= (1 << (fd % 8)))
#define FD_CLR(fd, fd_set_ptr) ((fd_set_ptr)->fds[fd / 8] &= ~(1 << (fd % 8)))
#define FD_ZERO(fd_set_ptr) (memset((uint8_t*)(fd_set_ptr), 0, sizeof(fd_set_t)))
#define FD_ISSET(fd, fd_set_ptr) ((fd_set_ptr)->fds[fd / 8] & (1 << (fd % 8)))

#endif // _KERNEL_LIBKERN_BITS_SELECT_H