/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <io/sockets/local_socket.h>
#include <libkern/bits/errno.h>
#include <libkern/libkern.h>
#include <libkern/log.h>
#include <mem/kmalloc.h>
#include <tasking/proc.h>
#include <tasking/tasking.h>

// #define LOCAL_SOCKET_DEBUG

static file_ops_t local_socket_ops = {
    .can_read = local_socket_can_read,
    .can_write = local_socket_can_write,
    .read = local_socket_read,
    .write = local_socket_write,
    .open = 0,
    .truncate = 0,
    .create = 0,
    .unlink = 0,
    .getdents = 0,
    .lookup = 0,
    .mkdir = 0,
    .rmdir = 0,
    .fstat = 0,
    .ioctl = 0,
    .mmap = 0,
};

int local_socket_create(int type, int protocol, file_descriptor_t* fd)
{
    return socket_create(PF_LOCAL, type, protocol, fd, &local_socket_ops);
}

bool local_socket_can_read(dentry_t* dentry, uint32_t start)
{
    socket_t* sock_entry = (socket_t*)dentry;
    return ringbuffer_space_to_read_with_custom_start(&sock_entry->buffer, start) != 0;
}

int local_socket_read(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len)
{
    socket_t* sock_entry = (socket_t*)dentry;
    uint32_t read = ringbuffer_read_with_start(&sock_entry->buffer, start, buf, len);
    return read;
}

/* Each process has it's own start when reading from a local socket.
   We ignore their offsets and write always, hope all readers could
   read all needed data. */
bool local_socket_can_write(dentry_t* dentry, uint32_t start)
{
    return true;
}

int local_socket_write(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len)
{
    socket_t* sock_entry = (socket_t*)dentry;
    uint32_t written = ringbuffer_write_ignore_bounds(&sock_entry->buffer, buf, len);
    return 0;
}

int local_socket_bind(file_descriptor_t* sock, char* path, uint32_t len)
{
    proc_t* p = RUNNING_THREAD->process;

    char* name = vfs_helper_split_path_with_name(path, strlen(path));
    dentry_t* location;
    if (vfs_resolve_path_start_from(p->cwd, path, &location) < 0) {
        vfs_helper_restore_full_path_after_split(path, name);
        kfree(name);
        return -ENOENT;
    }

    mode_t file_mode = S_IFSOCK | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
    vfs_create(location, name, strlen(name), file_mode);

    dentry_t* bind_dentry;
    int res = vfs_resolve_path_start_from(location, name, &bind_dentry);
    if (res < 0) {
#ifdef LOCAL_SOCKET_DEBUG
        log_error("Bind: can't find path to file : %d pid\n", p->pid);
#endif
        dentry_put(location);
        return res;
    }
    dentry_put(location);

    res = vfs_open(bind_dentry, &sock->sock_entry->bind_file, O_RDONLY);
    if (res < 0) {
#ifdef LOCAL_SOCKET_DEBUG
        log_error("Bind: can't open file [%d] : %d pid\n", -res, p->pid);
#endif
        return res;
    }
#ifdef LOCAL_SOCKET_DEBUG
    log("Bind local socket at %x : %d pid", sock->sock_entry, p->pid);
#endif
    sock->sock_entry->bind_file.dentry->sock = socket_duplicate(sock->sock_entry);
    vfs_helper_restore_full_path_after_split(path, name);
    return 0;
}

int local_socket_connect(file_descriptor_t* sock, char* path, uint32_t len)
{
    proc_t* p = RUNNING_THREAD->process;

    dentry_t* bind_dentry;
    int res = vfs_resolve_path_start_from(p->cwd, path, &bind_dentry);
    if (res < 0) {
#ifdef LOCAL_SOCKET_DEBUG
        log_error("Connect: can't find path to file %s : %d pid\n", path, p->pid);
#endif
        return res;
    }
    if ((bind_dentry->inode->mode & S_IFSOCK) == 0) {
#ifdef LOCAL_SOCKET_DEBUG
        log_error("Connect: file not a socket : %d pid\n", p->pid);
#endif
        return -ENOTSOCK;
    }

    if (!bind_dentry->sock) {
        return -EBADF;
    }
    sock->sock_entry = socket_duplicate(bind_dentry->sock);
    sock->offset = bind_dentry->sock->buffer.end; /* Starting to read from the end */
#ifdef LOCAL_SOCKET_DEBUG
    log("Connected to local socket at %x : %d pid", bind_dentry->sock, p->pid);
#endif
    return 0;
}