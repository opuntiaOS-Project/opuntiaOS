#ifndef _LIBC_SYS_WAIT_H
#define _LIBC_SYS_WAIT_H

#include <bits/sys/wait.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

int wait(int pid);
int waitpid(int pid, int* status, int options);

__END_DECLS

#endif // _LIBC_SYS_WAIT_H