#ifndef _LIBC_SYSDEP_H
#define _LIBC_SYSDEP_H

#include <bits/syscalls.h>
#include <errno.h>
#include <stdint.h>
#include <sys/cdefs.h>

__BEGIN_DECLS

static inline intptr_t _syscall_impl(sysid_t sysid, intptr_t p1, intptr_t p2, intptr_t p3, intptr_t p4, intptr_t p5)
{
    intptr_t ret;
#ifdef __i386__
    asm volatile("push %%ebx;movl %2,%%ebx;int $0x80;pop %%ebx"
                 : "=a"(ret)
                 : "0"(sysid), "r"((intptr_t)(p1)), "c"((intptr_t)(p2)), "d"((intptr_t)(p3)), "S"((intptr_t)(p4)), "D"((intptr_t)(p5))
                 : "memory");
#elif __x86_64__
    asm volatile(
        "movq %1, %%rax;\
        movq %2, %%rdi;\
        movq %3, %%rsi;\
        movq %4, %%rdx;\
        movq %5, %%r10;\
        movq %6, %%r8;\
        int $0x80;\
        movq %%rax, %0;"
        : "=r"(ret)
        : "r"(sysid), "r"((intptr_t)(p1)), "r"((intptr_t)(p2)), "r"((intptr_t)(p3)), "r"((intptr_t)(p4)), "r"((intptr_t)(p5))
        : "memory", "rax", "rdi", "rsi", "rdx", "r10", "r8");
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
        : "r"(sysid), "r"((intptr_t)(p1)), "r"((intptr_t)(p2)), "r"((intptr_t)(p3)), "r"((intptr_t)(p4)), "r"((intptr_t)(p5))
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
        : "r"(sysid), "r"((intptr_t)(p1)), "r"((intptr_t)(p2)), "r"((intptr_t)(p3)), "r"((intptr_t)(p4)), "r"((intptr_t)(p5))
        : "memory", "x0", "x1", "x2", "x3", "x4", "x8");
#endif
    return ret;
}

#define DO_SYSCALL_0(type) _syscall_impl(type, 0, 0, 0, 0, 0)
#define DO_SYSCALL_1(type, a) _syscall_impl(type, (intptr_t)a, 0, 0, 0, 0)
#define DO_SYSCALL_2(type, a, b) _syscall_impl(type, (intptr_t)a, (intptr_t)b, 0, 0, 0)
#define DO_SYSCALL_3(type, a, b, c) _syscall_impl(type, (intptr_t)a, (intptr_t)b, (intptr_t)c, 0, 0)
#define DO_SYSCALL_4(type, a, b, c, d) _syscall_impl(type, (intptr_t)a, (intptr_t)b, (intptr_t)c, (intptr_t)d, 0)
#define DO_SYSCALL_5(type, a, b, c, d, e) _syscall_impl(type, (intptr_t)a, (intptr_t)b, (intptr_t)c, (intptr_t)d, (intptr_t)e);
#define RETURN_WITH_ERRNO(res, on_suc, on_fail) \
    do {                                        \
        if ((intptr_t)res < 0) {                \
            set_errno(res);                     \
            return (on_fail);                   \
        }                                       \
        set_errno(0);                           \
        return on_suc;                          \
    } while (0);

__END_DECLS

#endif // _LIBC_SYSDEP_H