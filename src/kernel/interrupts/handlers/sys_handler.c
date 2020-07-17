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

/* From Linux 4.14.0 headers. */
/* https://chromium.googlesource.com/chromiumos/docs/+/master/constants/syscalls.md#x86-32_bit */

static inline void set_return(trapframe_t* tf, uint32_t val)
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
        sys_write,
        sys_open,
        sys_close, // 6
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

/* TODO: copying to/from user! */
void sys_read(trapframe_t* tf)
{
    file_descriptor_t* fd = (file_descriptor_t*)proc_get_fd(tasking_get_active_proc(), (int)param1);
    if (!fd) {
        set_return(tf, -1);
        return;
    }
    int res = vfs_read(fd, (uint8_t*)param2, fd->offset, (uint32_t)param3);
    set_return(tf, res);
}

/* TODO: copying to/from user! */
void sys_write(trapframe_t* tf)
{
    file_descriptor_t* fd = (file_descriptor_t*)proc_get_fd(tasking_get_active_proc(), (int)param1);
    if (!fd) {
        set_return(tf, -1);
        return;
    }
    int res = vfs_write(fd, (uint8_t*)param2, fd->offset, (uint32_t)param3);
    set_return(tf, res);
}

void sys_open(trapframe_t* tf)
{
    file_descriptor_t* fd = proc_get_free_fd(tasking_get_active_proc());
    dentry_t* file;
    if (vfs_resolve_path((char*)param1, &file) < 0) {
        set_return(tf, -1);
        return;
    }
    int res = vfs_open(file, fd);
    dentry_put(file);
    set_return(tf, res);
}

void sys_close(trapframe_t* tf)
{
    file_descriptor_t* fd = proc_get_fd(tasking_get_active_proc(), param1);
    set_return(tf, vfs_close(fd));
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
