/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <drivers/display.h>
#include <errno.h>
#include <io/sockets/local_socket.h>
#include <tasking/proc.h>
#include <tasking/tasking.h>

static file_ops_t local_socket_ops = {
    local_socket_can_read,
    local_socket_can_write,
    local_socket_read,
    local_socket_write,
    0,
    0,
    0,
    0,
    0
};

int local_socket_create(int type, int protocol, file_descriptor_t* fd)
{
    return socket_create(PF_LOCAL, type, protocol, fd, &local_socket_ops);
}

bool local_socket_can_read(dentry_t* dentry)
{
    socket_t* sock_entry = (socket_t*)dentry;
    return ringbuffer_space_to_read(&sock_entry->buffer) != 0;
}

int local_socket_read(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len)
{
    socket_t* sock_entry = (socket_t*)dentry;
    uint32_t read = ringbuffer_read(&sock_entry->buffer, buf, len);
    return !(read == len);
}

bool local_socket_can_write(dentry_t* dentry)
{
    socket_t* sock_entry = (socket_t*)dentry;
    return ringbuffer_space_to_write(&sock_entry->buffer) != 0;
}

int local_socket_write(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len)
{
    socket_t* sock_entry = (socket_t*)dentry;
    uint32_t written = ringbuffer_write(&sock_entry->buffer, buf, len);
    return !(written == len);
}

int local_socket_bind(file_descriptor_t* sock, char* name, uint32_t len)
{
    proc_t* p = RUNNIG_PROC;

    uint32_t file_mode = EXT2_S_IFSOCK | EXT2_S_IRUSR | EXT2_S_IXUSR | EXT2_S_IRGRP | EXT2_S_IXGRP | EXT2_S_IROTH | EXT2_S_IXOTH;
    vfs_create(p->cwd, name, len, file_mode);
    dentry_t* bind_dentry;
    int res = vfs_resolve_path_start_from(p->cwd, name, &bind_dentry);
    if (res < 0) {
        kprintf("Bind: can't find path to file\n");
        return res;
    }
    res = vfs_open(bind_dentry, &sock->sock_entry->bind_file);
    if (res < 0) {
        kprintf("Bind: can't open file\n");
        return res;
    }

    kprintf("Bind to %x", sock->sock_entry);
    sock->sock_entry->bind_file.dentry->sock = sock->sock_entry;
    return 0;
}

int local_socket_connect(file_descriptor_t* sock, char* name, uint32_t len)
{
    proc_t* p = RUNNIG_PROC;

    dentry_t* bind_dentry;
    int res = vfs_resolve_path_start_from(p->cwd, name, &bind_dentry);
    if (res < 0) {
        kprintf("Connect: can't find path to file\n");
        return res;
    }
    if ((bind_dentry->inode->mode & EXT2_S_IFSOCK) == 0) {
        kprintf("Connect: file not a socket\n");
        return -ENOTSOCK;
    }

    if (!bind_dentry->sock) {
        return -EBADF;
    }
    sock->sock_entry = bind_dentry->sock;
    kprintf("Connected to %x", bind_dentry->sock);
    return 0;
}