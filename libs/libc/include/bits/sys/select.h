#ifndef _LIBC_BITS_SYS_SELECT_H
#define _LIBC_BITS_SYS_SELECT_H

#include <sys/types.h>

#define FD_SETSIZE 32

struct fd_set {
    uint8_t fds[FD_SETSIZE / 8];
};
typedef struct fd_set fd_set_t;

#define FD_SET(fd, fd_set_ptr) ((fd_set_ptr)->fds[fd / 8] |= (1 << (fd % 8)))
#define FD_CLR(fd, fd_set_ptr) ((fd_set_ptr)->fds[fd / 8] &= ~(1 << (fd % 8)))
#define FD_ZERO(fd_set_ptr) (memset((uint8_t*)(fd_set_ptr), 0, sizeof(fd_set_t)))
#define FD_ISSET(fd, fd_set_ptr) ((fd_set_ptr)->fds[fd / 8] & (1 << (fd % 8)))

#endif // _LIBC_BITS_SYS_SELECT_H