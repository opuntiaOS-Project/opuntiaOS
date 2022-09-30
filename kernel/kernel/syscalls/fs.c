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
    char __user* path = (char __user*)SYSCALL_VAR1(tf);
    if (!umem_validate_str(path, USER_STR_MAXLEN)) {
        return_with_val(-EINVAL);
    }

    uint32_t flags = SYSCALL_VAR2(tf);
    size_t path_len = strlen((char*)path);
    char* kpath = umem_bring_to_kernel(path, path_len + 1);

    // Only permission flags
    mode_t mode = SYSCALL_VAR3(tf) & 0777;

    if (TEST_FLAG(flags, O_CREAT)) {
        char* kname = vfs_helper_split_path_with_name(kpath, path_len);
        if (!kname) {
            kfree(kpath);
            return_with_val(-EINVAL);
        }
        size_t name_len = strlen(kname);

        path_t rootdir;
        if (vfs_resolve_path_start_from(&p->cwd, kpath, &rootdir) < 0) {
            kfree(kname);
            kfree(kpath);
            return_with_val(-ENOENT);
        }

        int err = vfs_create(&rootdir, kname, name_len, mode, p->uid, p->gid);
        if (err && TEST_FLAG(flags, O_EXCL)) {
            path_put(&rootdir);
            kfree(kname);
            kfree(kpath);
            return_with_val(err);
        }

        vfs_helper_restore_full_path_after_split(kpath, kname);
        path_put(&rootdir);
        kfree(kname);
    }

    path_t filepath;
    if (vfs_resolve_path_start_from(&p->cwd, kpath, &filepath) < 0) {
        return_with_val(-ENOENT);
    }
    int err = vfs_open(&filepath, fd, flags);
    path_put(&filepath);
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

    int res = vfs_read(fd, (uint8_t __user*)SYSCALL_VAR2(tf), (size_t)SYSCALL_VAR3(tf));
    return_with_val(res);
}

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

    uint8_t __user* buf = (uint8_t __user*)SYSCALL_VAR2(tf);
    size_t len = (size_t)SYSCALL_VAR3(tf);
    int res = vfs_write(fd, buf, len);
    return_with_val(res);
}

void sys_lseek(trapframe_t* tf)
{
    file_descriptor_t* fd = proc_get_fd(RUNNING_THREAD->process, (int)SYSCALL_VAR1(tf));
    if (!fd) {
        return_with_val(-EBADF);
    }

    if (fd->file->type != FTYPE_FILE) {
        return_with_val(-ESPIPE);
    }

    int whence = SYSCALL_VAR3(tf);
    dentry_t* dentry = file_dentry(fd->file);

    switch (whence) {
    case SEEK_SET:
        fd->offset = SYSCALL_VAR2(tf);
        break;
    case SEEK_CUR:
        fd->offset += SYSCALL_VAR2(tf);
        break;
    case SEEK_END:
        fd->offset = dentry->inode->size - SYSCALL_VAR2(tf);
        break;
    default:
        return_with_val(-EINVAL);
    }

    if (fd->offset >= dentry->inode->size) {
        return_with_val(-EOVERFLOW);
    }

    return_with_val(0);
}

void sys_unlink(trapframe_t* tf)
{
    proc_t* p = RUNNING_THREAD->process;
    char __user* path = (char __user*)SYSCALL_VAR1(tf);
    char* kpath = umem_bring_to_kernel_str(path, USER_STR_MAXLEN);
    if (!kpath) {
        return_with_val(-EINVAL);
    }

    path_t filepath;
    if (vfs_resolve_path_start_from(&p->cwd, kpath, &filepath) < 0) {
        kfree(kpath);
        return_with_val(-ENOENT);
    }

    int ret = vfs_unlink(&filepath);

    path_put(&filepath);
    kfree(kpath);
    return_with_val(ret);
}

void sys_creat(trapframe_t* tf)
{
    uintptr_t tmp_storage_2 = SYSCALL_VAR2(tf);
    uintptr_t tmp_storage_3 = SYSCALL_VAR3(tf);
    SYSCALL_VAR2(tf) = O_CREAT | O_WRONLY | O_TRUNC;
    SYSCALL_VAR3(tf) = SYSCALL_VAR2(tf);
    sys_open(tf);
    uintptr_t result = return_val;
    SYSCALL_VAR2(tf) = tmp_storage_2;
    SYSCALL_VAR3(tf) = tmp_storage_3;
    return_with_val(result);
}

void sys_fstat(trapframe_t* tf)
{
    file_descriptor_t* fd = proc_get_fd(RUNNING_THREAD->process, (int)SYSCALL_VAR1(tf));
    stat_t __user* stat = (stat_t __user*)SYSCALL_VAR2(tf);
    if (!fd) {
        return_with_val(-EBADF);
    }
    if (!stat) {
        return_with_val(-EINVAL);
    }

    stat_t kstat = { 0 };
    int res = vfs_fstat(fd, &kstat);
    umem_copy_to_user(stat, &kstat, sizeof(stat_t));
    return_with_val(res);
}

void sys_fsync(trapframe_t* tf)
{
    file_descriptor_t* fd = proc_get_fd(RUNNING_THREAD->process, (int)SYSCALL_VAR1(tf));
    if (!fd) {
        return_with_val(-EBADF);
    }
    if (fd->file->type != FTYPE_FILE) {
        return_with_val(-EINVAL);
    }
    dentry_flush(fd->file->dentry);
    return_with_val(0);
}

void sys_mkdir(trapframe_t* tf)
{
    proc_t* p = RUNNING_THREAD->process;
    char __user* path = (char __user*)SYSCALL_VAR1(tf);
    if (!umem_validate_str(path, USER_STR_MAXLEN)) {
        return_with_val(-EINVAL);
    }

    size_t path_len = strlen((char*)path);
    char* kpath = umem_bring_to_kernel(path, path_len + 1);
    char* kname = vfs_helper_split_path_with_name(kpath, path_len);
    if (!kname) {
        kfree(kpath);
        return_with_val(-EINVAL);
    }
    size_t name_len = strlen(kname);

    path_t dirpath;
    if (vfs_resolve_path_start_from(&p->cwd, kpath, &dirpath) < 0) {
        kfree(kname);
        kfree(kpath);
        return_with_val(-ENOENT);
    }

    mode_t dir_mode = S_IFDIR | S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
    int res = vfs_mkdir(&dirpath, kname, name_len, dir_mode, p->uid, p->gid);
    path_put(&dirpath);
    kfree(kname);
    kfree(kpath);
    return_with_val(res);
}

void sys_rmdir(trapframe_t* tf)
{
    proc_t* p = RUNNING_THREAD->process;
    char __user* path = (char __user*)SYSCALL_VAR1(tf);
    char* kpath = umem_bring_to_kernel_str(path, USER_STR_MAXLEN);
    if (!kpath) {
        return_with_val(-EINVAL);
    }

    path_t dirpath;
    if (vfs_resolve_path_start_from(&p->cwd, kpath, &dirpath) < 0) {
        kfree(kpath);
        return_with_val(-ENOENT);
    }

    int res = vfs_rmdir(&dirpath);
    path_put(&dirpath);
    kfree(kpath);
    return_with_val(res);
}

void sys_chdir(trapframe_t* tf)
{
    char __user* path = (char __user*)SYSCALL_VAR1(tf);
    char* kpath = umem_bring_to_kernel_str(path, USER_STR_MAXLEN);
    if (!kpath) {
        return_with_val(-EINVAL);
    }

    int ret = proc_chdir(RUNNING_THREAD->process, kpath);
    kfree(kpath);
    return_with_val(ret);
}

void sys_chmod(trapframe_t* tf)
{
    proc_t* p = RUNNING_THREAD->process;
    char __user* path = (char __user*)SYSCALL_VAR1(tf);
    mode_t mode = (mode_t)SYSCALL_VAR2(tf);
    char* kpath = umem_bring_to_kernel_str(path, USER_STR_MAXLEN);
    if (!kpath) {
        return_with_val(-EINVAL);
    }

    path_t filepath;
    int err = vfs_resolve_path_start_from(&p->cwd, path, &filepath);
    if (err) {
        kfree(kpath);
        return_with_val(-ENOENT);
    }

    err = vfs_chmod(&filepath, mode);
    path_put(&filepath);
    kfree(kpath);
    return_with_val(err);
}

void sys_fchmod(trapframe_t* tf)
{
    proc_t* p = RUNNING_THREAD->process;
    file_descriptor_t* fd = (file_descriptor_t*)proc_get_fd(p, (size_t)SYSCALL_VAR1(tf));
    mode_t mode = (mode_t)SYSCALL_VAR2(tf);

    int err = vfs_fchmod(fd, mode);
    return_with_val(err);
}

void sys_getcwd(trapframe_t* tf)
{
    proc_t* p = RUNNING_THREAD->process;
    char __user* buf = (char __user*)SYSCALL_VAR1(tf);
    size_t len = (size_t)SYSCALL_VAR2(tf);

    int req_len = vfs_get_absolute_path(&p->cwd, NULL, 0);
    if (len < req_len) {
        return_with_val(-EFAULT);
    }

    char* kbuf = (char*)kmalloc(req_len + 1);
    req_len = vfs_get_absolute_path(&p->cwd, (char*)kbuf, req_len);
    umem_copy_to_user(buf, kbuf, req_len + 1);
    return_with_val(0);
}

void sys_getdents(trapframe_t* tf)
{
    proc_t* p = RUNNING_THREAD->process;
    file_descriptor_t* fd = (file_descriptor_t*)proc_get_fd(p, (size_t)SYSCALL_VAR1(tf));
    int read = vfs_getdents(fd, (uint8_t __user*)SYSCALL_VAR2(tf), SYSCALL_VAR3(tf));
    return_with_val(read);
}

void sys_select(trapframe_t* tf)
{
    fd_set_t kreadfds, kwritefds, kexceptfds;
    timeval_t ktimeout;
    proc_t* p = RUNNING_THREAD->process;
    file_descriptor_t* fd;

    int nfds = SYSCALL_VAR1(tf);
    if (nfds < 0 || nfds > FD_SETSIZE) {
        return_with_val(-EINVAL);
    }

    fd_set_t __user* ureadfds = (fd_set_t __user*)SYSCALL_VAR2(tf);
    fd_set_t __user* uwritefds = (fd_set_t __user*)SYSCALL_VAR3(tf);
    fd_set_t __user* uexceptfds = (fd_set_t __user*)SYSCALL_VAR4(tf);
    timeval_t __user* utimeout = (timeval_t __user*)SYSCALL_VAR5(tf);

    fd_set_t* readfds = &kreadfds;
    fd_set_t* writefds = &kwritefds;
    fd_set_t* exceptfds = &kexceptfds;
    timeval_t* timeout = &ktimeout;

    if (ureadfds) {
        umem_get_user(&kreadfds, ureadfds);
    } else {
        readfds = NULL;
    }
    if (uwritefds) {
        umem_get_user(&kwritefds, uwritefds);
    } else {
        writefds = NULL;
    }
    if (uexceptfds) {
        umem_get_user(&kexceptfds, uexceptfds);
    } else {
        exceptfds = NULL;
    }
    if (utimeout) {
        umem_get_user(&ktimeout, utimeout);
    } else {
        timeout = NULL;
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
            if (fd->file->ops->can_read && fd->file->ops->can_read(fd->file, fd->offset)) {
                FD_SET(i, readfds);
            }
        }
        if (writefds && FD_ISSET(i, &(RUNNING_THREAD->blocker_data.select.writefds))) {
            if (fd->file->ops->can_write && fd->file->ops->can_write(fd->file, fd->offset)) {
                FD_SET(i, writefds);
            }
        }
    }

    if (ureadfds) {
        umem_put_user(kreadfds, ureadfds);
    }
    if (uwritefds) {
        umem_put_user(kwritefds, uwritefds);
    }
    if (uexceptfds) {
        umem_put_user(kexceptfds, uexceptfds);
    }
    if (utimeout) {
        umem_put_user(ktimeout, utimeout);
    }
    return_with_val(0);
}

void sys_mmap(trapframe_t* tf)
{
    proc_t* p = RUNNING_THREAD->process;
    mmap_params_t kparams;
    mmap_params_t __user* params = (mmap_params_t __user*)SYSCALL_VAR1(tf);
    umem_get_user(&kparams, params);

    bool map_shared = ((kparams.flags & MAP_SHARED) > 0);
    bool map_anonymous = ((kparams.flags & MAP_ANONYMOUS) > 0);
    bool map_private = ((kparams.flags & MAP_PRIVATE) > 0);
    bool map_stack = ((kparams.flags & MAP_STACK) > 0);
    bool map_fixed = ((kparams.flags & MAP_FIXED) > 0);

    bool map_exec = ((kparams.prot & PROT_EXEC) > 0);
    bool map_read = ((kparams.prot & PROT_READ) > 0);
    bool map_write = ((kparams.prot & PROT_WRITE) > 0);

    memzone_t* zone;

    if (map_private && map_shared) {
        return_with_val(-EINVAL);
    }
    if (!map_private && !map_shared) {
        return_with_val(-EINVAL);
    }

    if (map_stack) {
        zone = memzone_new_random_backward(p->address_space, kparams.size);
    } else if (map_anonymous) {
        zone = memzone_new_random(p->address_space, kparams.size);
    } else {
        file_descriptor_t* fd = proc_get_fd(p, kparams.fd);
        // TODO: Check for read access to the file.
        if (!fd) {
            return_with_val(-EBADFD);
        }
        zone = vfs_mmap(fd, &kparams);
    }

    if (!zone) {
        return_with_val(-ENOMEM);
    }

    zone->type |= ZONE_TYPE_MAPPED;
    if (map_read) {
        zone->mmu_flags |= MMU_FLAG_PERM_READ;
    }
    if (map_write) {
        zone->mmu_flags |= MMU_FLAG_PERM_WRITE;
    }
    if (map_exec) {
        zone->mmu_flags |= MMU_FLAG_PERM_EXEC;
    }

    return_with_val(zone->vaddr);
}

void sys_munmap(trapframe_t* tf)
{
    proc_t* p = RUNNING_THREAD->process;
    uintptr_t ptr = (uintptr_t)SYSCALL_VAR1(tf);
    uintptr_t len = (uintptr_t)SYSCALL_VAR2(tf);

    memzone_t* zone = memzone_find(p->address_space, ptr);
    if (!zone) {
        return_with_val(-EFAULT);
    }

    memzone_t* mzone = memzone_split(p->address_space, zone, ptr);
    if (mzone) {
        mzone->file = file_duplicate(zone->file);
        mzone->file_offset = zone->file_offset;
        mzone->file_size = zone->file_size;
    } else {
        mzone = zone;
    }

    memzone_t* rzone = memzone_split(p->address_space, mzone, ptr + len);
    if (rzone) {
        rzone->file = file_duplicate(zone->file);
        rzone->file_offset = zone->file_offset;
        rzone->file_size = zone->file_size;
    }

    if (!TEST_FLAG(mzone->type, ZONE_TYPE_MAPPED)) {
        return_with_val(-EPERM);
    }

    if (TEST_FLAG(mzone->type, ZONE_TYPE_MAPPED_FILE_PRIVATLY) || TEST_FLAG(mzone->type, ZONE_TYPE_MAPPED_FILE_SHAREDLY)) {
        return_with_val(vfs_munmap(p, mzone));
    }
    return_with_val(0);
}

void sys_dup(trapframe_t* tf)
{
    proc_t* p = RUNNING_THREAD->process;
    file_descriptor_t* fd = proc_get_fd(RUNNING_THREAD->process, (int)SYSCALL_VAR1(tf));
    if (!fd) {
        return_with_val(-EBADF);
    }

    file_descriptor_t* newfd = proc_get_free_fd(p);
    if (!newfd) {
        return_with_val(-EBADF);
    }

    int err = proc_copy_fd(fd, newfd);
    ASSERT(!err);

    return_with_val(proc_get_fd_id(p, newfd));
}

void sys_dup2(trapframe_t* tf)
{
    proc_t* p = RUNNING_THREAD->process;
    file_descriptor_t* fd = proc_get_fd(RUNNING_THREAD->process, (int)SYSCALL_VAR1(tf));
    if (!fd) {
        return_with_val(-EBADF);
    }

    file_descriptor_t* newfd = proc_get_fd(RUNNING_THREAD->process, (int)SYSCALL_VAR2(tf));
    if (!newfd) {
        return_with_val(-EBADF);
    }

    int err = proc_copy_fd(fd, newfd);
    ASSERT(!err);

    return_with_val(proc_get_fd_id(p, newfd));
}