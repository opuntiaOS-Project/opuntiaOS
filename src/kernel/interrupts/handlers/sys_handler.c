/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <sys_handler.h>
#include <tasking/tasking.h>

#define param1 (tf->ebx)
#define param2 (tf->ecx)
#define param3 (tf->edx)

/* 32 bit Linux-like syscalls */

static inline void set_return(trapframe_t* tf, int val)
{
    tf->eax = val;
}

void sys_handler(trapframe_t* tf)
{
    const void* syscalls[] = {
        sys_restart_syscall,
        sys_exit,
        sys_fork,
        sys_read,
        sys_open,
        sys_close, // 5
        sys_exec,
        sys_sigaction,
        sys_sigreturn,
        sys_raise,
    };
    void (*callee)(trapframe_t*) = (void*)syscalls[tf->eax];
    callee(tf);
}

void sys_restart_syscall(trapframe_t* tf)
{
    kprintd(tf->ebx);
}

void sys_exit(trapframe_t* tf)
{
    tasking_exit(tf);
}

void sys_fork(trapframe_t* tf)
{
    tasking_fork(tf);
}

void sys_read(trapframe_t* tf)
{
}

int sys_open(trapframe_t* tf)
{
    file_descriptor_t* fd = tasking_get_free_fd(tasking_get_active_proc());
    dentry_t* file;
    if (vfs_resolve_path((char*)param1, &file) < 0) {
        return -1;
    }
    int res = vfs_open(file, fd);
    dentry_put(file);
    return res;
}

int sys_close(trapframe_t* tf)
{
    file_descriptor_t* fd = tasking_get_fd(tasking_get_active_proc(), param1);
    return vfs_close(fd);
}

void sys_exec(trapframe_t* tf)
{
    tasking_exec(tf);
}

void sys_sigaction(trapframe_t* tf)
{
    set_return(tf, signal_set_handler(tasking_get_active_proc(), (int)param1, (void*)param2));
}

void sys_sigreturn(trapframe_t* tf)
{
    signal_restore_proc_after_handling_signal(tasking_get_active_proc());
}

void sys_raise(trapframe_t* tf)
{
    signal_set_pending(tasking_get_active_proc(), (int)param1);
    signal_dispatch_pending(tasking_get_active_proc());
}
