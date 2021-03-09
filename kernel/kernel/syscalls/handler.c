/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
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

/* From Linux 4.14.0 headers. */
/* https://chromium.googlesource.com/chromiumos/docs/+/master/constants/syscalls.md#x86-32_bit */

static const void* syscalls[] = {
    [SYSRESTART] = sys_restart_syscall,
    [SYSEXIT] = sys_exit,
    [SYSFORK] = sys_fork,
    [SYSREAD] = sys_read,
    [SYSWRITE] = sys_write,
    [SYSOPEN] = sys_open,
    [SYSCLOSE] = sys_close,
    [SYSWAITPID] = sys_waitpid,
    [SYSCREAT] = sys_creat,
    [SYSLINK] = sys_none, // sys_link
    [SYSUNLINK] = sys_unlink,
    [SYSEXEC] = sys_exec,
    [SYSCHDIR] = sys_chdir,
    [SYSSIGACTION] = sys_sigaction,
    [SYSSIGRETURN] = sys_sigreturn, // When this is moved, change signal_caller.s for now.
    [SYSRAISE] = sys_raise,
    [SYSLSEEK] = sys_lseek,
    [SYSGETPID] = sys_getpid,
    [SYSKILL] = sys_kill,
    [SYSMKDIR] = sys_mkdir,
    [SYSRMDIR] = sys_rmdir,
    [SYSMMAP] = sys_mmap,
    [SYSMUNMAP] = sys_munmap,
    [SYSSOCKET] = sys_socket,
    [SYSBIND] = sys_bind,
    [SYSCONNECT] = sys_connect,
    [SYSGETDENTS] = sys_getdents,
    [SYSIOCTL] = sys_ioctl,
    [SYSSETPGID] = sys_setpgid,
    [SYSGETPGID] = sys_getpgid,
    [SYSPTHREADCREATE] = sys_create_thread,
    [SYSSLEEP] = sys_sleep,
    [SYSSELECT] = sys_select,
    [SYSFSTAT] = sys_fstat,
    [SYSSCHEDYIELD] = sys_sched_yield,
    [SYSUNAME] = sys_uname,
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
    void (*callee)(trapframe_t*) = (void*)syscalls[sys_id];
    callee(tf);
    system_enable_interrupts_only_counter();
}

void sys_restart_syscall(trapframe_t* tf)
{
}

void sys_none(trapframe_t* tf) { }
