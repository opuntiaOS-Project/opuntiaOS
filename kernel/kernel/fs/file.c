/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <fs/vfs.h>
#include <io/sockets/socket.h>
#include <libkern/kassert.h>
#include <libkern/lock.h>
#include <libkern/log.h>
#include <libkern/mem.h>
#include <mem/kmalloc.h>
#include <syscalls/handlers.h>

// #define FILE_DEBUG

static file_t* file_alloc()
{
    return (file_t*)kmalloc(sizeof(file_t));
}

file_t* file_init_dentry(dentry_t* dentry)
{
    file_t* file = file_alloc();
    file->count = 1;
    file->type = FTYPE_FILE;
    file->dentry = dentry_duplicate(dentry);
    file->ops = &dentry->ops->file;
    spinlock_init(&file->lock);
    return file;
}

file_t* file_init_dentry_move(dentry_t* dentry)
{
    file_t* file = file_alloc();
    file->count = 1;
    file->type = FTYPE_FILE;
    file->dentry = dentry;
    file->ops = &dentry->ops->file;
    spinlock_init(&file->lock);
    return file;
}

file_t* file_init_socket(socket_t* socket, file_ops_t* ops)
{
    file_t* file = file_alloc();
    file->count = 1;
    file->type = FTYPE_SOCKET;
    extern socket_t* socket_duplicate(socket_t * sock);
    file->socket = socket_duplicate(socket);
    file->ops = ops;
    spinlock_init(&file->lock);
    return file;
}

file_t* file_init_socket_move(socket_t* socket, file_ops_t* ops)
{
    file_t* file = file_alloc();
    file->count = 1;
    file->type = FTYPE_SOCKET;
    file->socket = socket;
    file->ops = ops;
    spinlock_init(&file->lock);
    return file;
}

file_t* file_duplicate(file_t* file)
{
    spinlock_acquire(&file->lock);
    file->count++;
    spinlock_release(&file->lock);
    return file;
}

file_t* file_duplicate_locked(file_t* file)
{
    file->count++;
    return file;
}

static void file_put_impl_locked(file_t* file)
{
    switch (file->type) {
    case FTYPE_FILE:
        dentry_put(file->dentry);
        break;

    case FTYPE_SOCKET:
        socket_put(file->socket);
        break;

    default:
        break;
    }

    file->dentry = NULL;
    file->ops = NULL;
    kfree(file);
}

static void file_put_locked(file_t* file)
{
    ASSERT(file->count > 0);
    file->count--;

    if (file->count == 0) {
        file_put_impl_locked(file);
    }
}

void file_put(file_t* file)
{
    spinlock_acquire(&file->lock);
    file_put_locked(file);
    spinlock_release(&file->lock);
}
