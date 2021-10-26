/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <io/shared_buffer/shared_buffer.h>
#include <io/sockets/local_socket.h>
#include <libkern/bits/errno.h>
#include <libkern/libkern.h>
#include <libkern/log.h>
#include <platform/generic/syscalls/params.h>
#include <syscalls/handlers.h>
#include <tasking/tasking.h>

void sys_socket(trapframe_t* tf)
{
    proc_t* p = RUNNING_THREAD->process;
    int domain = SYSCALL_VAR1(tf);
    int type = SYSCALL_VAR2(tf);
    int protocol = SYSCALL_VAR3(tf);

    file_descriptor_t* fd = proc_get_free_fd(p);
    if (!fd) {
        return_with_val(-1);
    }

    if (domain == PF_LOCAL) {
        int res = local_socket_create(type, protocol, fd);
        if (!res) {
            return_with_val(proc_get_fd_id(p, fd));
        }
        return_with_val(res);
    }
    return_with_val(-1);
}

void sys_bind(trapframe_t* tf)
{
    proc_t* p = RUNNING_THREAD->process;
    int sockfd = SYSCALL_VAR1(tf);
    char* name = (char*)SYSCALL_VAR2(tf);
    uint32_t len = (uint32_t)SYSCALL_VAR3(tf);

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
    proc_t* p = RUNNING_THREAD->process;
    int sockfd = SYSCALL_VAR1(tf);
    char* name = (char*)SYSCALL_VAR2(tf);
    uint32_t len = (uint32_t)SYSCALL_VAR3(tf);

    file_descriptor_t* sfd = proc_get_fd(p, sockfd);
    if (sfd->type != FD_TYPE_SOCKET || !sfd->sock_entry) {
        return_with_val(-EBADF);
    }

    if (sfd->sock_entry->domain == PF_LOCAL) {
        return_with_val(local_socket_connect(sfd, name, len));
    }

    return_with_val(-EFAULT);
}

void sys_ioctl(trapframe_t* tf)
{
    proc_t* p = RUNNING_THREAD->process;
    file_descriptor_t* fd = proc_get_fd(p, SYSCALL_VAR1(tf));

    if (!fd) {
        return_with_val(-EBADF);
    }

    if (!fd->dentry->ops->file.ioctl) {
        return_with_val(-EACCES);
    }
    return_with_val(fd->dentry->ops->file.ioctl(fd->dentry, SYSCALL_VAR2(tf), SYSCALL_VAR3(tf)));
}

void sys_shbuf_create(trapframe_t* tf)
{
    uint8_t** buffer = (uint8_t**)SYSCALL_VAR1(tf);
    size_t size = SYSCALL_VAR2(tf);
    return_with_val(shared_buffer_create(buffer, size));
}

void sys_shbuf_get(trapframe_t* tf)
{
    int id = SYSCALL_VAR1(tf);
    uint8_t** buffer = (uint8_t**)SYSCALL_VAR2(tf);
    return_with_val(shared_buffer_get(id, buffer));
}

void sys_shbuf_free(trapframe_t* tf)
{
    int id = SYSCALL_VAR1(tf);
    return_with_val(shared_buffer_free(id));
}