/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libkern/bits/errno.h>
#include <libkern/libkern.h>
#include <libkern/log.h>
#include <mem/kmalloc.h>
#include <platform/generic/syscalls/params.h>
#include <platform/generic/system.h>
#include <syscalls/handlers.h>
#include <tasking/cpu.h>

static const void* syscalls[] = {
    [SYS_RESTART_SYSCALL] = sys_restart_syscall,
    [SYS_EXIT] = sys_exit,
    [SYS_FORK] = sys_fork,
    [SYS_READ] = sys_read,
    [SYS_WRITE] = sys_write,
    [SYS_OPEN] = sys_open,
    [SYS_CLOSE] = sys_close,
    [SYS_WAITPID] = sys_waitpid,
    [SYS_CREAT] = sys_creat,
    [SYS_LINK] = sys_none, // sys_link
    [SYS_UNLINK] = sys_unlink,
    [SYS_EXECVE] = sys_exec,
    [SYS_CHDIR] = sys_chdir,
    [SYS_CHMOD] = sys_chmod,
    [SYS_FCHMOD] = sys_fchmod,
    [SYS_GETCWD] = sys_getcwd,
    [SYS_SIGACTION] = sys_sigaction,
    [SYS_SIGRETURN] = sys_sigreturn,
    [SYS_GETTIMEOFDAY] = sys_gettimeofday,
    [SYS_LSEEK] = sys_lseek,
    [SYS_GETPID] = sys_getpid,
    [SYS_GETUID] = sys_getuid,
    [SYS_SETUID] = sys_setuid,
    [SYS_SETGID] = sys_setgid,
    [SYS_SETREUID] = sys_setreuid,
    [SYS_SETREGID] = sys_setregid,
    [SYS_KILL] = sys_kill,
    [SYS_MKDIR] = sys_mkdir,
    [SYS_RMDIR] = sys_rmdir,
    [SYS_MMAP] = sys_mmap,
    [SYS_MUNMAP] = sys_munmap,
    [SYS_DUP] = sys_dup,
    [SYS_DUP2] = sys_dup2,
    [SYS_SOCKET] = sys_socket,
    [SYS_BIND] = sys_bind,
    [SYS_CONNECT] = sys_connect,
    [SYS_GETDENTS] = sys_getdents,
    [SYS_IOCTL] = sys_ioctl,
    [SYS_SETPGID] = sys_setpgid,
    [SYS_GETPGID] = sys_getpgid,
    [SYS_PTHREAD_CREATE] = sys_create_thread,
    [SYS_NANOSLEEP] = sys_sleep,
    [SYS_PTRACE] = sys_ptrace,
    [SYS_SELECT] = sys_select,
    [SYS_FSTAT] = sys_fstat,
    [SYS_FSYNC] = sys_fsync,
    [SYS_SCHED_YIELD] = sys_sched_yield,
    [SYS_UNAME] = sys_uname,
    [SYS_CLOCK_GETTIME] = sys_clock_gettime,
    [SYS_CLOCK_SETTIME] = sys_none,
    [SYS_CLOCK_GETRES] = sys_none,
    [SYS_NICE] = sys_nice,
    [SYS_SHBUF_CREATE] = sys_shbuf_create,
    [SYS_SHBUF_GET] = sys_shbuf_get,
    [SYS_SHBUF_FREE] = sys_shbuf_free,
};

#if defined(__i386__) || defined(__x86_64__)
int ksyscall_impl(intptr_t id, intptr_t a, intptr_t b, intptr_t c, intptr_t d)
{
#ifndef PREEMPT_KERNEL
    system_disable_interrupts();
#endif
    trapframe_t* tf;
    trapframe_t tf_on_stack;
    tf = &tf_on_stack;
    SYSCALL_ID(tf) = id;
    SYSCALL_VAR1(tf) = a;
    SYSCALL_VAR2(tf) = b;
    SYSCALL_VAR3(tf) = c;
    sys_handler(tf);

#ifndef PREEMPT_KERNEL
    // This hack has to be here, when a context switching happens
    // during a syscall (e.g. when block occurs). The hack will start
    // interrupts again after it has become a running thread.
    system_enable_interrupts();
#endif
    return SYSCALL_VAR1(tf);
}
#elif __arm__
int ksyscall_impl(intptr_t id, intptr_t a, intptr_t b, intptr_t c, intptr_t d)
{
    int ret;
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
        : "r"(id), "r"((intptr_t)(a)), "r"((intptr_t)(b)), "r"((intptr_t)(c)), "r"((intptr_t)(d)), "r"((intptr_t)(0))
        : "memory", "r0", "r1", "r2", "r3", "r4", "r7");
    return ret;
}
#elif __aarch64__
int ksyscall_impl(intptr_t id, intptr_t a, intptr_t b, intptr_t c, intptr_t d)
{
    int ret;
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
        : "r"(id), "r"((intptr_t)(a)), "r"((intptr_t)(b)), "r"((intptr_t)(c)), "r"((intptr_t)(d)), "r"((intptr_t)(0))
        : "memory", "x0", "x1", "x2", "x3", "x4", "x8");
    return ret;
}
#elif defined(__riscv) && (__riscv_xlen == 64)
int ksyscall_impl(intptr_t id, intptr_t a, intptr_t b, intptr_t c, intptr_t d)
{
    int ret;
    asm volatile(
        "mv a7, %1;\
        mv a0, %2;\
        mv a1, %3;\
        mv a2, %4;\
        mv a3, %5;\
        mv a4, %6;\
        ecall;\
        mv %0, a0;"
        : "=r"(ret)
        : "r"(id), "r"((intptr_t)(a)), "r"((intptr_t)(b)), "r"((intptr_t)(c)), "r"((intptr_t)(d)), "r"((intptr_t)(0))
        : "memory", "a0", "a1", "a2", "a3", "a4", "a7");
    return ret;
}
#endif

void sys_handler(trapframe_t* tf)
{
#ifdef PREEMPT_KERNEL
    system_enable_interrupts_no_counter();
#else
    system_disable_interrupts();
#endif

    cpu_state_t prev_cpu_state = cpu_enter_kernel_space();
    void (*callee)(trapframe_t*) = (void*)syscalls[SYSCALL_ID(tf)];
    callee(tf);
    cpu_set_state(prev_cpu_state);

#ifndef PREEMPT_KERNEL
    system_enable_interrupts_only_counter();
#endif
}

void sys_restart_syscall(trapframe_t* tf)
{
}

void sys_none(trapframe_t* tf) { }
