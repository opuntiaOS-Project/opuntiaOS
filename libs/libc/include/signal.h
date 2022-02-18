#ifndef _LIBC_SIGNAL_H
#define _LIBC_SIGNAL_H

#include <bits/signal.h>
#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

int kill(pid_t pid, int sig);
int sigaction(int signo, sighandler_t callback);
sighandler_t signal(int signo, sighandler_t callback);
int raise(int sig);

__END_DECLS

#endif // _LIBC_SIGNAL_H