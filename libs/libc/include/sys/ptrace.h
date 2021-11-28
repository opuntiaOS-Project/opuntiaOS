#ifndef _LIBC_SYS_PTRACE_H
#define _LIBC_SYS_PTRACE_H

#include <bits/sys/ptrace.h>
#include <stddef.h>
#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

int ptrace(ptrace_request_t request, pid_t pid, void* addr, void* data);

__END_DECLS

#endif // _LIBC_SYS_PTRACE_H