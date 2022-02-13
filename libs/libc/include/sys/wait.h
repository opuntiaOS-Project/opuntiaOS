#ifndef _LIBC_SYS_WAIT_H
#define _LIBC_SYS_WAIT_H

#include <fcntl.h>
#include <stddef.h>
#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

int wait(int pid);
int waitpid(int pid, int* status);

__END_DECLS

#endif // _LIBC_SYS_WAIT_H