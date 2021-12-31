/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libkern/bits/errno.h>
#include <libkern/libkern.h>
#include <libkern/log.h>
#include <mem/kmalloc.h>
#include <platform/generic/syscalls/params.h>
#include <syscalls/handlers.h>
#include <syscalls/interruptible_area.h>
#include <tasking/tasking.h>

void sys_open(trapframe_t* tf)
{
    proc_t* p = RUNNING_THREAD->process;
    file_descriptor_t* fd = proc_get_free_fd(p);
    dentry_t* file;
    const char* path = (char*)SYSCALL_VAR1(tf);
    char* kpath = 0;
    if (!str_validate_len(path, 128)) {
        return_with_val(-EINVAL);
    }

    uint32_t flags = SYSCALL_VAR2(tf);
    size_t path_len = strlen(path);
    kpath = kmem_bring_to_kernel(path, path_len + 1);

    // Only permission flags
    mode_t mode = SYSCALL_VAR3(tf) & 0x777;

    if (TEST_FLAG(flags, O_CREAT)) {
        char* kname = vfs_helper_split_path_with_name(kpath, path_len);
        if (!kname) {
            kfree(kpath);
            return_with_val(-EINVAL);
        }
        size_t name_len = strlen(kname);

        dentry_t* dir;
        if (vfs_resolve_path_start_from(p->cwd, kpath, &dir) < 0) {
            kfree(kname);
            kfree(kpath);
            return_with_val(-ENOENT);
        }

        int err = vfs_create(dir, kname, name_len, mode, p->uid, p->gid);
        if (err && TEST_FLAG(flags, O_EXCL)) {
            dentry_put(dir);
            kfree(kname);
            kfree(kpath);
            return_with_val(err);
        }

        vfs_helper_restore_full_path_after_split(kpath, kname);
        dentry_put(dir);
        kfree(kname);
    }

    if (vfs_resolve_path_start_from(p->cwd, kpath, &file) < 0) {
        return_with_val(-ENOENT);
    }
    int err = vfs_open(file, fd, flags);
    dentry_put(file);
    if (err) {
        return_with_val(err);
    }
    return_with_val(proc_get_fd_id(p, fd));
}

void sys_close(trapframe_t* tf)
{
    file_descriptor_t* fd = proc_get_fd(RUNNING_THREAD->process, SYSCALL_VAR1(tf));
    if (!fd) {
        return_with_val(-EBADF);
    }
    return_with_val(vfs_close(fd));
}

void sys_read(trapframe_t* tf)
{
    file_descriptor_t* fd = proc_get_fd(RUNNING_THREAD->process, (int)SYSCALL_VAR1(tf));
    if (!fd) {
        return_with_val(-EBADF);
    }
    if (TEST_FLAG(fd->flags, O_DIRECTORY)) {
        return_with_val(-EISDIR);
    }
    if (!TEST_FLAG(fd->flags, O_RDONLY)) {
        return_with_val(-EBADF);
    }

    init_read_blocker(RUNNING_THREAD, fd);

    int res = vfs_read(fd, (uint8_t*)SYSCALL_VAR2(tf), (uint32_t)SYSCALL_VAR3(tf));
    return_with_val(res);
}

/* TODO: copying to/from user! */
void sys_write(trapframe_t* tf)
{
    file_descriptor_t* fd = proc_get_fd(RUNNING_THREAD->process, (int)SYSCALL_VAR1(tf));
    if (!fd) {
        return_with_val(-EBADF);
    }
    if (!TEST_FLAG(fd->flags, O_WRONLY)) {
        return_with_val(-EBADF);
    }

    init_write_blocker(RUNNING_THREAD, fd);

    int res = vfs_write(fd, (uint8_t*)SYSCALL_VAR2(tf), (uint32_t)SYSCALL_VAR3(tf));
    return_with_val(res);
}

void sys_lseek(trapframe_t* tf)
{
    file_descriptor_t* fd = proc_get_fd(RUNNING_THREAD->process, (int)SYSCALL_VAR1(tf));
    if (!fd) {
        return_with_val(-EBADF);
    }

    int whence = SYSCALL_VAR3(tf);

    switch (whence) {
    case SEEK_SET:
        fd->offset = SYSCALL_VAR2(tf);
        break;
    case SEEK_CUR:
        fd->offset += SYSCALL_VAR2(tf);
        break;
    case SEEK_END:
        fd->offset = fd->dentry->inode->size - SYSCALL_VAR2(tf);
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
    proc_t* p = RUNNING_THREAD->process;
    const char* path = (char*)SYSCALL_VAR1(tf);
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
    uint32_t tmp_storage_2 = SYSCALL_VAR2(tf);
    uint32_t tmp_storage_3 = SYSCALL_VAR3(tf);
    SYSCALL_VAR2(tf) = O_CREAT | O_WRONLY | O_TRUNC;
    SYSCALL_VAR3(tf) = SYSCALL_VAR2(tf);
    sys_open(tf);
    uint32_t result = return_val;
    SYSCALL_VAR2(tf) = tmp_storage_2;
    SYSCALL_VAR3(tf) = tmp_storage_3;
    return_with_val(result);
}

void sys_fstat(trapframe_t* tf)
{
    file_descriptor_t* fd = proc_get_fd(RUNNING_THREAD->process, (int)SYSCALL_VAR1(tf));
    fstat_t* stat = (fstat_t*)SYSCALL_VAR2(tf);
    if (!fd) {
        return_with_val(-EBADF);
    }
    if (!stat) {
        return_with_val(-EINVAL);
    }
    int res = vfs_fstat(fd, stat);
    return_with_val(res);
}

void sys_fsync(trapframe_t* tf)
{
    file_descriptor_t* fd = proc_get_fd(RUNNING_THREAD->process, (int)SYSCALL_VAR1(tf));
    dentry_flush(fd->dentry);
    return_with_val(0);
}

void sys_mkdir(trapframe_t* tf)
{
    proc_t* p = RUNNING_THREAD->process;
    const char* path = (char*)SYSCALL_VAR1(tf);
    char* kpath = 0;
    if (!str_validate_len(path, 128)) {
        return_with_val(-EINVAL);
    }
    size_t path_len = strlen(path);
    kpath = kmem_bring_to_kernel(path, path_len + 1);
    char* kname = vfs_helper_split_path_with_name(kpath, path_len);
    if (!kname) {
        kfree(kpath);
        return_with_val(-EINVAL);
    }
    size_t name_len = strlen(kname);

    dentry_t* dir;
    if (vfs_resolve_path_start_from(p->cwd, kpath, &dir) < 0) {
        kfree(kname);
        kfree(kpath);
        return_with_val(-ENOENT);
    }

    mode_t dir_mode = S_IFDIR | S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
    int res = vfs_mkdir(dir, kname, name_len, dir_mode, p->uid, p->gid);
    kfree(kname);
    kfree(kpath);
    return_with_val(res);
}

void sys_rmdir(trapframe_t* tf)
{
    proc_t* p = RUNNING_THREAD->process;
    const char* path = (char*)SYSCALL_VAR1(tf);
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
    const char* path = (char*)SYSCALL_VAR1(tf);
    return_with_val(proc_chdir(RUNNING_THREAD->process, path));
}

void sys_getdents(trapframe_t* tf)
{
    proc_t* p = RUNNING_THREAD->process;
    file_descriptor_t* fd = (file_descriptor_t*)proc_get_fd(p, (uint32_t)SYSCALL_VAR1(tf));
    int read = vfs_getdents(fd, (uint8_t*)SYSCALL_VAR2(tf), SYSCALL_VAR3(tf));
    return_with_val(read);
}

void sys_select(trapframe_t* tf)
{
    proc_t* p = RUNNING_THREAD->process;
    file_descriptor_t* fd;

    int nfds = SYSCALL_VAR1(tf);
    fd_set_t* readfds = (fd_set_t*)SYSCALL_VAR2(tf);
    fd_set_t* writefds = (fd_set_t*)SYSCALL_VAR3(tf);
    fd_set_t* exceptfds = (fd_set_t*)SYSCALL_VAR4(tf);
    timeval_t* timeout = (timeval_t*)SYSCALL_VAR5(tf);
    if (nfds < 0 || nfds > FD_SETSIZE) {
        return_with_val(-EINVAL);
    }

    for (int i = 0; i < nfds; i++) {
        if ((readfds && FD_ISSET(i, readfds)) || (writefds && FD_ISSET(i, writefds)) || (exceptfds && FD_ISSET(i, exceptfds))) {
            if (!proc_get_fd(p, i)) {
                return_with_val(-EBADF);
            }
        }
    }

    init_select_blocker(RUNNING_THREAD, nfds, readfds, writefds, exceptfds, timeout);

    if (readfds) {
        FD_ZERO(readfds);
    }
    if (writefds) {
        FD_ZERO(writefds);
    }
    if (exceptfds) {
        FD_ZERO(exceptfds);
    }

    for (int i = 0; i < nfds; i++) {
        fd = proc_get_fd(p, i);
        if (readfds && FD_ISSET(i, &(RUNNING_THREAD->blocker_data.select.readfds))) {
            if (fd->ops->can_read && fd->ops->can_read(fd->dentry, fd->offset)) {
                FD_SET(i, readfds);
            }
        }
        if (writefds && FD_ISSET(i, &(RUNNING_THREAD->blocker_data.select.writefds))) {
            if (fd->ops->can_write && fd->ops->can_write(fd->dentry, fd->offset)) {
                FD_SET(i, writefds);
            }
        }
    }

    return_with_val(0);
}

void sys_mmap(trapframe_t* tf)
{
    proc_t* p = RUNNING_THREAD->process;
    mmap_params_t* params = (mmap_params_t*)SYSCALL_VAR1(tf);

    bool map_shared = ((params->flags & MAP_SHARED) > 0);
    bool map_anonymous = ((params->flags & MAP_ANONYMOUS) > 0);
    bool map_private = ((params->flags & MAP_PRIVATE) > 0);
    bool map_stack = ((params->flags & MAP_STACK) > 0);
    bool map_fixed = ((params->flags & MAP_FIXED) > 0);

    bool map_exec = ((params->prot & PROT_EXEC) > 0);
    bool map_read = ((params->prot & PROT_READ) > 0);
    bool map_write = ((params->prot & PROT_WRITE) > 0);

    memzone_t* zone;

    if (map_private && map_shared) {
        return_with_val(-EINVAL);
    }
    if (!map_private && !map_shared) {
        return_with_val(-EINVAL);
    }

    if (map_stack) {
        zone = memzone_new_random_backward(p, params->size);
    } else if (map_anonymous) {
        zone = memzone_new_random(p, params->size);
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

    zone->type |= ZONE_TYPE_MAPPED;
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
    proc_t* p = RUNNING_THREAD->process;
    void* ptr = (void*)SYSCALL_VAR1(tf);

    memzone_t* zone = memzone_find(p, (uint32_t)ptr);
    if (!zone) {
        return_with_val(-EFAULT);
    }

    if (!TEST_FLAG(zone->type, ZONE_TYPE_MAPPED)) {
        return_with_val(-EPERM);
    }

    if (TEST_FLAG(zone->type, ZONE_TYPE_MAPPED_FILE_PRIVATLY) || TEST_FLAG(zone->type, ZONE_TYPE_MAPPED_FILE_SHAREDLY)) {
        return_with_val(vfs_munmap(p, zone));
    }

    // TODO: Split or remove zone
    return_with_val(0);
}