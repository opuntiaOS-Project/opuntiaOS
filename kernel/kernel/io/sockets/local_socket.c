/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
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
    .open = NULL,
    .truncate = NULL,
    .create = NULL,
    .unlink = NULL,
    .getdents = NULL,
    .lookup = NULL,
    .mkdir = NULL,
    .rmdir = NULL,
    .fstat = NULL,
    .ioctl = NULL,
    .mmap = NULL,
};

int local_socket_create(int type, int protocol, file_descriptor_t* fd)
{
    return socket_create(PF_LOCAL, type, protocol, fd, &local_socket_ops);
}

bool local_socket_can_read(file_t* file, size_t start)
{
    socket_t* sock_entry = file_socket_assert(file);
    return sync_ringbuffer_space_to_read_from(&sock_entry->buffer, start) != 0;
}

int local_socket_read(file_t* file, void __user* buf, size_t start, size_t len)
{
    socket_t* sock_entry = file_socket_assert(file);
    int read = sync_ringbuffer_read_user_from(&sock_entry->buffer, start, buf, len);
    return read;
}

bool local_socket_can_write(file_t* file, size_t start)
{
    // Each process has it's own start when reading from a local socket.
    // We ignore their offsets and write always, hope all readers could
    // read all needed data.
    return true;
}

int local_socket_write(file_t* file, void __user* buf, size_t start, size_t len)
{
    socket_t* sock_entry = file_socket_assert(file);
    size_t written = sync_ringbuffer_write_user_ignore_bounds(&sock_entry->buffer, buf, len);
    return 0;
}

int local_socket_bind(file_descriptor_t* sock, char* path, size_t len)
{
    spinlock_acquire(&sock->file->lock);
    socket_t* sock_entry = file_socket_assert(sock->file);
    proc_t* p = RUNNING_THREAD->process;

    char* name = vfs_helper_split_path_with_name(path, len);
    path_t location;
    if (vfs_resolve_path_start_from(&p->cwd, path, &location) < 0) {
        vfs_helper_restore_full_path_after_split(path, name);
        kfree(name);
        spinlock_release(&sock->file->lock);
        return -ENOENT;
    }

    mode_t file_mode = S_IFSOCK | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    vfs_create(&location, name, strlen(name), file_mode, p->uid, p->gid);

    path_t bind_path;
    int res = vfs_resolve_path_start_from(&location, name, &bind_path);
    if (res < 0) {
#ifdef LOCAL_SOCKET_DEBUG
        log_error("Bind: can't find path to file : %d pid\n", p->pid);
#endif
        path_put(&location);
        spinlock_release(&sock->file->lock);
        return res;
    }
    path_put(&location);

#ifdef LOCAL_SOCKET_DEBUG
    log("Bind local socket at %x : %d pid", sock->file->socket, p->pid);
#endif

    bind_path.dentry->sock = socket_duplicate(sock_entry);
    sock_entry->bind_file = file_init_path(&bind_path);

    path_put(&bind_path);
    vfs_helper_restore_full_path_after_split(path, name);
    spinlock_release(&sock->file->lock);
    return 0;
}

int local_socket_connect(file_descriptor_t* sock, char* path, size_t len)
{
    proc_t* p = RUNNING_THREAD->process;

    path_t bind_path;
    int res = vfs_resolve_path_start_from(&p->cwd, path, &bind_path);
    if (res < 0) {
#ifdef LOCAL_SOCKET_DEBUG
        log_error("Connect: can't find path to file %s : %d pid\n", path, p->pid);
#endif
        return res;
    }
    if ((bind_path.dentry->inode->mode & S_IFSOCK) == 0) {
#ifdef LOCAL_SOCKET_DEBUG
        log_error("Connect: file not a socket : %d pid\n", p->pid);
#endif
        return -ENOTSOCK;
    }

    if (!bind_path.dentry->sock) {
        return -EBADF;
    }
    sock->flags = O_RDWR;
    sock->file = file_init_socket(bind_path.dentry->sock, &local_socket_ops);
    sock->offset = bind_path.dentry->sock->buffer.ringbuffer.end; // Starting to read from the end.
#ifdef LOCAL_SOCKET_DEBUG
    log("Connected to local socket at %x : %d pid", bind_dentry->sock, p->pid);
#endif
    return 0;
}