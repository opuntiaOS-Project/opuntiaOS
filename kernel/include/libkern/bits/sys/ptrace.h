#ifndef _KERNEL_LIBKERN_BITS_SYS_PTRACE_H
#define _KERNEL_LIBKERN_BITS_SYS_PTRACE_H

#include <libkern/types.h>

typedef int ptrace_request_t;
#define PTRACE_TRACEME (0x1)
#define PTRACE_CONT (0x2)
#define PTRACE_PEEKTEXT (0x3)
#define PTRACE_PEEKDATA (0x4)

#endif // _KERNEL_LIBKERN_BITS_SYS_PTRACE_H
