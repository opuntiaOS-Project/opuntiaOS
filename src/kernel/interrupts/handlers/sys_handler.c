/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <errno.h>
#include <io/sockets/local_socket.h>
#include <sys_handler.h>
#include <syscall_structs.h>
#include <tasking/sched.h>
#include <tasking/tasking.h>
#include <x86/common.h>

#define param1 (tf->ebx)
#define param2 (tf->ecx)
#define param3 (tf->edx)

/* From Linux 4.14.0 headers. */
/* https://chromium.googlesource.com/chromiumos/docs/+/master/constants/syscalls.md#x86-32_bit */

#define return_with_val(val) (tf->eax = val);return

static inline void set_return(trapframe_t* tf, uint32_t val)
{
    tf->eax = val;
}

void sys_handler(trapframe_t* tf)
{
    cli();
    const void* syscalls[] = {
        sys_restart_syscall,
        sys_exit,
        sys_fork,
        sys_read,
        sys_write,
        sys_open,
        sys_close,
        sys_waitpid,
        sys_none, // sys_creat
        sys_none, // sys_link
        sys_none, // sys_unlink
        sys_exec,
        sys_sigaction,
        sys_sigreturn, // When this is moved, change signal_caller.s for now.
        sys_raise,
        sys_mmap,
        sys_munmap,
        sys_socket,
        sys_bind,
        sys_connect,
    };
    void (*callee)(trapframe_t*) = (void*)syscalls[tf->eax];
    callee(tf);
    sti();
}

void sys_restart_syscall(trapframe_t* tf)
{
    kprintd(tf->ebx);
}

void sys_exit(trapframe_t* tf)
{
    tasking_exit((int)param1);
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

    /* If we can't read right now, let's block until we can */
    if (!vfs_can_read(fd, (uint8_t*)param2, fd->offset, (uint32_t)param3)) {
        init_read_blocker(tasking_get_active_proc(), fd);
        presched();
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
    proc_t* p = tasking_get_active_proc();
    file_descriptor_t* fd = proc_get_free_fd(p);
    dentry_t* file;
    if (vfs_resolve_path_start_from(p->cwd, (char*)param1, &file) < 0) {
        set_return(tf, -1);
        return;
    }
    int res = vfs_open(file, fd);
    dentry_put(file);
    if (!res) {
        set_return(tf, proc_get_fd_id(p, fd));
    } else {
        set_return(tf, res);
    }
}

void sys_close(trapframe_t* tf)
{
    file_descriptor_t* fd = proc_get_fd(tasking_get_active_proc(), param1);
    set_return(tf, vfs_close(fd));
}

void sys_waitpid(trapframe_t* tf)
{
    int ret = tasking_waitpid(param1);
    set_return(tf, ret);
}

void sys_exec(trapframe_t* tf)
{
    set_return(tf, tasking_exec((char*)param1, (const char**)param2, (const char**)param3));
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

void sys_mmap(trapframe_t* tf)
{
    mmap_params_t* params = (mmap_params_t*)param1;

    bool map_shared = ((params->flags & MAP_SHARED) > 0);
    bool map_anonymous = ((params->flags & MAP_ANONYMOUS) > 0);
    bool map_private = ((params->flags & MAP_PRIVATE) > 0);
    bool map_stack = ((params->flags & MAP_STACK) > 0);
    bool map_fixed = ((params->flags & MAP_FIXED) > 0);

    bool map_exec = ((params->prot & PROT_EXEC) > 0);
    bool map_read = ((params->prot & PROT_READ) > 0);
    bool map_write = ((params->prot & PROT_WRITE) > 0);

    proc_zone_t* zone = proc_new_random_zone(tasking_get_active_proc(), params->size);
    if (!zone) {
        set_return(tf, -ENOMEM);
        return;
    }

    if (map_read) {
        zone->flags |= ZONE_READABLE;
    }
    if (map_write) {
        zone->flags |= ZONE_WRITABLE;
    }
    if (map_exec) {
        zone->flags |= ZONE_EXECUTABLE;
    }

    if (map_anonymous) {

    } else {
        /* TODO: Add support for not MAP_ANONYMOUS. */
        set_return(tf, -EFAULT);
        return;
    }

    set_return(tf, zone->start);
}

void sys_munmap(trapframe_t* tf)
{
}

void sys_socket(trapframe_t* tf)
{
    proc_t* p = tasking_get_active_proc();
    int domain = param1;
    int type = param2;
    int protocol = param3;

    file_descriptor_t* fd = proc_get_free_fd(p);
    if (!fd) {
        return_with_val(-1);
    }

    if (domain == PF_LOCAL) {
        int res = local_socket_create(type, protocol, fd);
        if (!res) {
            return_with_val(proc_get_fd_id(p, fd));
        } else {
            return_with_val(res);
        }
    }
}

void sys_bind(trapframe_t* tf)
{
    proc_t* p = tasking_get_active_proc();
    int sockfd = param1;
    char* name = (char*)param2;
    uint32_t len = (uint32_t)param3;

    file_descriptor_t* sfd = proc_get_fd(p, sockfd);
    if (sfd->type != FD_TYPE_SOCKET || !sfd->sock_entry) {
        return_with_val(-EBADF);
    }

    if (sfd->sock_entry->domain == PF_LOCAL) {
        return_with_val(local_socket_bind(sfd, name, len));
    }
    
    return_with_val(0);
}

void sys_connect(trapframe_t* tf)
{
    proc_t* p = tasking_get_active_proc();
    int sockfd = param1;
    char* name = (char*)param2;
    uint32_t len = (uint32_t)param3;

    file_descriptor_t* sfd = proc_get_fd(p, sockfd);
    if (sfd->type != FD_TYPE_SOCKET || !sfd->sock_entry) {
        return_with_val(-EBADF);
    }

    if (sfd->sock_entry->domain == PF_LOCAL) {
        return_with_val(local_socket_connect(sfd, name, len));
    }

    return_with_val(-EFAULT);
}

void sys_none(trapframe_t* tf) { }