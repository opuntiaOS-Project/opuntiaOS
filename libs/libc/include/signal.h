#ifndef _LIBC_SIGNAL_H
#define _LIBC_SIGNAL_H

#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

int kill(pid_t pid, int sig);
int sigaction(int signo, void* callback);
int raise(int sig);

__END_DECLS

#endif // _LIBC_SIGNAL_H