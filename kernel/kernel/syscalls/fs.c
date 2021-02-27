/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libkern/errno.h>
#include <libkern/libkern.h>
#include <libkern/log.h>
#include <mem/kmalloc.h>
#include <platform/generic/syscalls/params.h>
#include <syscalls/handlers.h>
#include <tasking/tasking.h>

void sys_open(trapframe_t* tf)
{
    proc_t* p = RUNNIG_THREAD->process;
    file_descriptor_t* fd = proc_get_free_fd(p);
    const char* path = (char*)param1;
    char* kpath = 0;
    if (!str_validate_len(path, 128)) {
        return_with_val(-EINVAL);
    }
    int name_len = strlen(path);
    kpath = kmem_bring_to_kernel(path, name_len + 1);
    uint32_t flags = param2;

    mode_t mode = S_IFREG | S_IRWXU | S_IRWXG | S_IRWXO;
    dentry_t* file;

    if (flags & O_CREATE) {
        vfs_create(p->cwd, kpath, name_len, mode);
    }

    if (vfs_resolve_path_start_from(p->cwd, kpath, &file) < 0) {
        return_with_val(-ENOENT);
    }
    int res = vfs_open(file, fd, param2);
    dentry_put(file);
    if (!res) {
        return_with_val(proc_get_fd_id(p, fd));
    } else {
        return_with_val(res);
    }
}

void sys_close(trapframe_t* tf)
{
    file_descriptor_t* fd = proc_get_fd(RUNNIG_THREAD->process, param1);
    if (!fd) {
        return_with_val(-EBADF);
    }
    return_with_val(vfs_close(fd));
}

/* TODO: copying to/from user! */
void sys_read(trapframe_t* tf)
{
    file_descriptor_t* fd = proc_get_fd(RUNNIG_THREAD->process, (int)param1);
    if (!fd) {
        return_with_val(-EBADF);
    }

    init_read_blocker(RUNNIG_THREAD, fd);

    int res = vfs_read(fd, (uint8_t*)param2, (uint32_t)param3);
    return_with_val(res);
}

/* TODO: copying to/from user! */
void sys_write(trapframe_t* tf)
{
    file_descriptor_t* fd = proc_get_fd(RUNNIG_THREAD->process, (int)param1);
    if (!fd) {
        return_with_val(-EBADF);
    }

    init_write_blocker(RUNNIG_THREAD, fd);

    int res = vfs_write(fd, (uint8_t*)param2, (uint32_t)param3);
    return_with_val(res);
}

void sys_lseek(trapframe_t* tf)
{
    file_descriptor_t* fd = proc_get_fd(RUNNIG_THREAD->process, (int)param1);
    if (!fd) {
        return_with_val(-EBADF);
    }

    int whence = param3;

    switch (whence) {
    case SEEK_SET:
        fd->offset = param2;
        break;
    case SEEK_CUR:
        fd->offset += param2;
        break;
    case SEEK_END:
        fd->offset = fd->dentry->inode->size - param2;
        break;
    default:
        return_with_val(-EINVAL);
    }

    if (fd->offset >= fd->dentry->inode->size) {
        return_with_val(-EOVERFLOW);
    }

    return_with_val(0);
}

void sys_unlink(trapframe_t* tf)
{
    proc_t* p = RUNNIG_THREAD->process;
    const char* path = (char*)param1;
    char* kpath = 0;
    if (!str_validate_len(path, 128)) {
        return_with_val(-EINVAL);
    }
    int name_len = strlen(path);
    kpath = kmem_bring_to_kernel(path, name_len + 1);

    dentry_t* file;
    if (vfs_resolve_path_start_from(p->cwd, kpath, &file) < 0) {
        return_with_val(-ENOENT);
    }

    int ret = vfs_unlink(file);

    dentry_put(file);
    kfree(kpath);
    return_with_val(ret);
}

void sys_creat(trapframe_t* tf)
{
    proc_t* p = RUNNIG_THREAD->process;
    const char* path = (char*)param1;
    mode_t mode = param2;
    char* kpath = 0;
    if (!str_validate_len(path, 128)) {
        return_with_val(-EINVAL);
    }
    int name_len = strlen(path);
    kpath = kmem_bring_to_kernel(path, name_len + 1);

    int err = vfs_create(p->cwd, kpath, name_len, mode);
    if (err) {
        return_with_val(err);
    }

    /* Opening file */
    file_descriptor_t* fd = proc_get_free_fd(p);
    dentry_t* file;
    if (vfs_resolve_path_start_from(p->cwd, kpath, &file) < 0) {
        return_with_val(-1);
    }
    err = vfs_open(file, fd, O_WRONLY);
    dentry_put(file);
    if (err) {
        return_with_val(err);
    }
    return_with_val(proc_get_fd_id(p, fd));
}

void sys_fstat(trapframe_t* tf)
{
    file_descriptor_t* fd = proc_get_fd(RUNNIG_THREAD->process, (int)param1);
    fstat_t* stat = (fstat_t*)param2;
    if (!fd) {
        return_with_val(-EBADF);
    }
    if (!stat) {
        return_with_val(-EINVAL);
    }
    int res = vfs_fstat(fd, stat);
    return_with_val(res);
}

void sys_mkdir(trapframe_t* tf)
{
    proc_t* p = RUNNIG_THREAD->process;
    const char* path = (char*)param1;
    char* kpath = 0;
    if (!str_validate_len(path, 128)) {
        return_with_val(-EINVAL);
    }
    int name_len = strlen(path);
    kpath = kmem_bring_to_kernel(path, name_len + 1);

    // dentry_t* dir;
    // if (vfs_resolve_path_start_from(p->cwd, &dir) < 0) {
    //     return_with_val(-ENOENT);
    // }

    mode_t dir_mode = S_IFDIR | S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
    int res = vfs_mkdir(p->cwd, kpath, name_len, dir_mode);
    kfree(kpath);
    return_with_val(res);
}

void sys_rmdir(trapframe_t* tf)
{
    proc_t* p = RUNNIG_THREAD->process;
    const char* path = (char*)param1;
    char* kpath = 0;
    if (!str_validate_len(path, 128)) {
        return_with_val(-EINVAL);
    }
    int name_len = strlen(path);
    kpath = kmem_bring_to_kernel(path, name_len + 1);

    dentry_t* dir;
    if (vfs_resolve_path_start_from(p->cwd, kpath, &dir) < 0) {
        return_with_val(-ENOENT);
    }

    int res = vfs_rmdir(dir);
    dentry_put(dir);
    kfree(kpath);
    return_with_val(res);
}

void sys_chdir(trapframe_t* tf)
{
    /* proc lock */
    const char* path = (char*)param1;
    return_with_val(proc_chdir(RUNNIG_THREAD->process, path));
}

void sys_getdents(trapframe_t* tf)
{
    proc_t* p = RUNNIG_THREAD->process;
    file_descriptor_t* fd = (file_descriptor_t*)proc_get_fd(p, (uint32_t)param1);
    int read = vfs_getdents(fd, (uint8_t*)param2, param3);
    return_with_val(read);
}

void sys_select(trapframe_t* tf)
{
    proc_t* p = RUNNIG_THREAD->process;
    file_descriptor_t* fd;

    int nfds = param1;
    fd_set_t* readfds = (fd_set_t*)param2;
    fd_set_t* writefds = (fd_set_t*)param3;
    fd_set_t* exceptfds = (fd_set_t*)param4;
    timeval_t* timeout = (timeval_t*)param5;
    if (nfds < 0 || nfds > FD_SETSIZE) {
        return_with_val(-EINVAL);
    }

    for (int i = 0; i < nfds; i++) {
        if (FD_ISSET(i, readfds) || FD_ISSET(i, writefds) || FD_ISSET(i, exceptfds)) {
            if (!proc_get_fd(p, i)) {
                return_with_val(-EBADF);
            }
        }
    }

    init_select_blocker(RUNNIG_THREAD, nfds, readfds, writefds, exceptfds, timeout);

    if (readfds)
        FD_ZERO(readfds);
    if (writefds)
        FD_ZERO(writefds);
    if (exceptfds)
        FD_ZERO(exceptfds);

    for (int i = 0; i < nfds; i++) {
        fd = proc_get_fd(p, i);
        if (readfds && FD_ISSET(i, &(RUNNIG_THREAD->readfds))) {
            if (fd->ops->can_read && fd->ops->can_read(fd->dentry, fd->offset)) {
                FD_SET(i, readfds);
            }
        }
        if (writefds && FD_ISSET(i, &(RUNNIG_THREAD->writefds))) {
            if (fd->ops->can_write && fd->ops->can_write(fd->dentry, fd->offset)) {
                FD_SET(i, writefds);
            }
        }
    }

    return_with_val(0);
}

void sys_mmap(trapframe_t* tf)
{
    proc_t* p = RUNNIG_THREAD->process;
    mmap_params_t* params = (mmap_params_t*)param1;

    bool map_shared = ((params->flags & MAP_SHARED) > 0);
    bool map_anonymous = ((params->flags & MAP_ANONYMOUS) > 0);
    bool map_private = ((params->flags & MAP_PRIVATE) > 0);
    bool map_stack = ((params->flags & MAP_STACK) > 0);
    bool map_fixed = ((params->flags & MAP_FIXED) > 0);

    bool map_exec = ((params->prot & PROT_EXEC) > 0);
    bool map_read = ((params->prot & PROT_READ) > 0);
    bool map_write = ((params->prot & PROT_WRITE) > 0);

    proc_zone_t* zone;

    if (map_private && map_shared) {
        return_with_val(-EINVAL);
    }
    if (!map_private && !map_shared) {
        return_with_val(-EINVAL);
    }

    if (map_stack) {
        zone = proc_new_random_zone_backward(p, params->size);
    } else if (map_anonymous) {
        zone = proc_new_random_zone(p, params->size);
    } else {
        file_descriptor_t* fd = proc_get_fd(p, params->fd);
        /* TODO: Check for read access to the file */
        if (!fd) {
            return_with_val(-EBADFD);
        }
        zone = vfs_mmap(fd, params);
    }

    if (!zone) {
        return_with_val(-ENOMEM);
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

    return_with_val(zone->start);
}

void sys_munmap(trapframe_t* tf)
{
}