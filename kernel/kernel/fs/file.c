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

static file_t* file_init_dentry(dentry_t* dentry)
{
    file_t* file = file_alloc();
    file->count = 1;
    file->type = FTYPE_FILE;
    file->dentry = dentry_duplicate(dentry);
    file->ops = &dentry->ops->file;
    spinlock_init(&file->lock);
    return file;
}

file_t* file_init_path(const path_t* path)
{
    // Both fp->dentry and fp->path.dentry point to the same dentry,
    // so only one time the counter is incremented.
    file_t* fp = file_init_dentry(path->dentry);
    fp->path = *path;
    return fp;
}

file_t* file_init_pseudo_dentry(dentry_t* dentry)
{
    // A pseudo dentry uses the similar structure to a typical dentry,
    // while path is not set and not valid.
    file_t* fp = file_init_dentry(dentry);
    fp->path = vfs_empty_path();
    return fp;
}

file_t* file_init_socket(socket_t* socket, file_ops_t* ops)
{
    file_t* file = file_alloc();
    file->count = 1;
    file->type = FTYPE_SOCKET;
    file->socket = socket_duplicate(socket);
    file->ops = ops;
    file->path = vfs_empty_path();
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

path_t path_duplicate(const path_t* path)
{
    dentry_duplicate(path->dentry);
    path_t a = *path;
    return a;
}

void path_put(path_t* path)
{
    dentry_put(path->dentry);
    path->dentry = NULL;
}