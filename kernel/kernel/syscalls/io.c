/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <io/shared_buffer/shared_buffer.h>
#include <io/sockets/local_socket.h>
#include <libkern/errno.h>
#include <libkern/libkern.h>
#include <libkern/log.h>
#include <platform/generic/syscalls/params.h>
#include <syscalls/handlers.h>
#include <tasking/tasking.h>

void sys_socket(trapframe_t* tf)
{
    proc_t* p = RUNNIG_THREAD->process;
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
    proc_t* p = RUNNIG_THREAD->process;
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
    proc_t* p = RUNNIG_THREAD->process;
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

void sys_ioctl(trapframe_t* tf)
{
    proc_t* p = RUNNIG_THREAD->process;
    file_descriptor_t* fd = proc_get_fd(p, param1);

    if (!fd) {
        return_with_val(-EBADF);
    }

    if (fd->dentry->ops->file.ioctl) {
        return_with_val(fd->dentry->ops->file.ioctl(fd->dentry, param2, param3));
    } else {
        return_with_val(-EACCES);
    }
}

void sys_shbuf_create(trapframe_t* tf)
{
    uint8_t** buffer = (uint8_t**)param1;
    size_t size = param2;
    return_with_val(shared_buffer_create(buffer, size));
}

void sys_shbuf_get(trapframe_t* tf)
{
    int id = param1;
    uint8_t** buffer = (uint8_t**)param2;
    return_with_val(shared_buffer_get(id, buffer));
}

void sys_shbuf_free(trapframe_t* tf)
{
    int id = param1;
    return_with_val(shared_buffer_free(id));
}