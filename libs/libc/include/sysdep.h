#ifndef _LIBC_SYSDEP_H
#define _LIBC_SYSDEP_H

#include <bits/syscalls.h>
#include <sys/cdefs.h>

__BEGIN_DECLS

static inline int _syscall_impl(sysid_t sysid, int p1, int p2, int p3, int p4, int p5)
{
    int ret;
#ifdef __i386__
    asm volatile("push %%ebx;movl %2,%%ebx;int $0x80;pop %%ebx"
                 : "=a"(ret)
                 : "0"(sysid), "r"((int)(p1)), "c"((int)(p2)), "d"((int)(p3)), "S"((int)(p4)), "D"((int)(p5))
                 : "memory");
#elif __arm__
    asm volatile(
        "mov r7, %1;\
        mov r0, %2;\
        mov r1, %3;\
        mov r2, %4;\
        mov r3, %5;\
        mov r4, %6;\
        swi 1;\
        mov %0, r0;"
        : "=r"(ret)
        : "r"(sysid), "r"((int)(p1)), "r"((int)(p2)), "r"((int)(p3)), "r"((int)(p4)), "r"((int)(p5))
        : "memory", "r0", "r1", "r2", "r3", "r4", "r7");
#endif
    return ret;
}

#define DO_SYSCALL_0(type) _syscall_impl(type, 0, 0, 0, 0, 0)
#define DO_SYSCALL_1(type, a) _syscall_impl(type, (int)a, 0, 0, 0, 0)
#define DO_SYSCALL_2(type, a, b) _syscall_impl(type, (int)a, (int)b, 0, 0, 0)
#define DO_SYSCALL_3(type, a, b, c) _syscall_impl(type, (int)a, (int)b, (int)c, 0, 0)

__END_DECLS

#endif // _LIBC_SYSDEP_H