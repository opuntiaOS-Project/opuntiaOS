#ifndef _LIBC_BITS_SYS_PTRACE_H
#define _LIBC_BITS_SYS_PTRACE_H

#include <stddef.h>
#include <sys/types.h>

typedef int ptrace_request_t;
#define PTRACE_TRACEME (0x1)
#define PTRACE_CONT (0x2)
#define PTRACE_PEEKTEXT (0x3)
#define PTRACE_PEEKDATA (0x4)

#endif // _LIBC_BITS_SYS_PTRACE_H