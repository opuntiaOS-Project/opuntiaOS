/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
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
    [SYS_SOCKET] = sys_socket,
    [SYS_BIND] = sys_bind,
    [SYS_CONNECT] = sys_connect,
    [SYS_GETDENTS] = sys_getdents,
    [SYS_IOCTL] = sys_ioctl,
    [SYS_SETPGID] = sys_setpgid,
    [SYS_GETPGID] = sys_getpgid,
    [SYS_PTHREAD_CREATE] = sys_create_thread,
    [SYS_NANOSLEEP] = sys_sleep,
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

#ifdef __i386__
int ksyscall_impl(int id, int a, int b, int c, int d)
{
    system_disable_interrupts();
    trapframe_t* tf;
    trapframe_t tf_on_stack;
    tf = &tf_on_stack;
    sys_id = id;
    param1 = a;
    param2 = b;
    param3 = c;
    sys_handler(tf);
    /* This hack has to be here, when a context switching happens
       during a syscall (e.g. when block occurs). The hack will start
       interrupts again after it has become a running thread. */
    cpu_enter_kernel_space();
    system_enable_interrupts();
    return param1;
}
#elif __arm__
int ksyscall_impl(int id, int a, int b, int c, int d)
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
        : "r"(id), "r"((int)(a)), "r"((int)(b)), "r"((int)(c)), "r"((int)(d)), "r"((int)(0))
        : "memory", "r0", "r1", "r2", "r3", "r4", "r7");
    return ret;
}
#endif

void sys_handler(trapframe_t* tf)
{
    system_disable_interrupts();
    cpu_enter_kernel_space();
    void (*callee)(trapframe_t*) = (void*)syscalls[sys_id];
    callee(tf);
    cpu_leave_kernel_space();
    system_enable_interrupts_only_counter();
}

void sys_restart_syscall(trapframe_t* tf)
{
}

void sys_none(trapframe_t* tf) { }
