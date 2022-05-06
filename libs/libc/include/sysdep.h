#ifndef _LIBC_SYSDEP_H
#define _LIBC_SYSDEP_H

#include <bits/syscalls.h>
#include <errno.h>
#include <stddef.h>
#include <sys/cdefs.h>

__BEGIN_DECLS

static inline size_t _syscall_impl(sysid_t sysid, size_t p1, size_t p2, size_t p3, size_t p4, size_t p5)
{
    size_t ret;
#ifdef __i386__
    asm volatile("push %%ebx;movl %2,%%ebx;int $0x80;pop %%ebx"
                 : "=a"(ret)
                 : "0"(sysid), "r"((size_t)(p1)), "c"((size_t)(p2)), "d"((size_t)(p3)), "S"((size_t)(p4)), "D"((size_t)(p5))
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
        : "r"(sysid), "r"((size_t)(p1)), "r"((size_t)(p2)), "r"((size_t)(p3)), "r"((size_t)(p4)), "r"((size_t)(p5))
        : "memory", "r0", "r1", "r2", "r3", "r4", "r7");
#elif __aarch64__
    asm volatile(
        "mov x8, %x1;\
        mov x0, %x2;\
        mov x1, %x3;\
        mov x2, %x4;\
        mov x3, %x5;\
        mov x4, %x6;\
        svc 1;\
        mov %x0, x0;"
        : "=r"(ret)
        : "r"(sysid), "r"((size_t)(p1)), "r"((size_t)(p2)), "r"((size_t)(p3)), "r"((size_t)(p4)), "r"((size_t)(p5))
        : "memory", "x0", "x1", "x2", "x3", "x4", "x8");
#endif
    return ret;
}

#define DO_SYSCALL_0(type) _syscall_impl(type, 0, 0, 0, 0, 0)
#define DO_SYSCALL_1(type, a) _syscall_impl(type, (size_t)a, 0, 0, 0, 0)
#define DO_SYSCALL_2(type, a, b) _syscall_impl(type, (size_t)a, (size_t)b, 0, 0, 0)
#define DO_SYSCALL_3(type, a, b, c) _syscall_impl(type, (size_t)a, (size_t)b, (size_t)c, 0, 0)
#define DO_SYSCALL_4(type, a, b, c, d) _syscall_impl(type, (size_t)a, (size_t)b, (size_t)c, (size_t)d, 0)
#define DO_SYSCALL_5(type, a, b, c, d, e) _syscall_impl(type, (size_t)a, (size_t)b, (size_t)c, (size_t)d, (size_t)e);
#define RETURN_WITH_ERRNO(res, on_suc, on_fail) \
    do {                                        \
        if ((size_t)res < 0) {                  \
            set_errno(res);                     \
            return (on_fail);                   \
        }                                       \
        set_errno(0);                           \
        return on_suc;                          \
    } while (0);

__END_DECLS

#endif // _LIBC_SYSDEP_H