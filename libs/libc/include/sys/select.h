#ifndef _LIBC_SYS_SELECT_H
#define _LIBC_SYS_SELECT_H

#include <bits/sys/select.h>
#include <bits/time.h>
#include <stddef.h>
#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

int select(int nfds, fd_set_t* readfds, fd_set_t* writefds, fd_set_t* exceptfds, timeval_t* timeout);

__END_DECLS

#endif // _LIBC_SYS_SELECT_H