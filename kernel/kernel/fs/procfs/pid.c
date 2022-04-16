/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <fs/procfs/procfs.h>
#include <fs/vfs.h>
#include <libkern/bits/errno.h>
#include <libkern/libkern.h>
#include <mem/kmalloc.h>
#include <tasking/tasking.h>

/**
 * inode: xxxxPPPPPPPPPPBBBBBBBBBBBBBBBBBB
 * x - level bits
 * P - pid bits
 * B - bits of files at this level
 */
#define PROCFS_PID_LEVEL 2

/* PID */
int procfs_pid_getdents(dentry_t* dir, void __user* buf, off_t* offset, size_t len);
int procfs_pid_lookup(const path_t* path, const char* name, size_t len, path_t* result);

/* FILES */
static bool procfs_pid_memstat_can_read(file_t* file, size_t start);
static int procfs_pid_memstat_read(file_t* file, void __user* buf, size_t start, size_t len);

static bool procfs_pid_exe_can_read(file_t* file, size_t start);
static int procfs_pid_exe_read(file_t* file, void __user* buf, size_t start, size_t len);

/**
 * DATA
 */

const file_ops_t procfs_pid_ops = {
    .getdents = procfs_pid_getdents,
    .lookup = procfs_pid_lookup,
};

const file_ops_t procfs_pid_memstat_ops = {
    .can_read = procfs_pid_memstat_can_read,
    .read = procfs_pid_memstat_read,
};

const file_ops_t procfs_pid_stat_ops = {
    .can_read = procfs_pid_memstat_can_read,
    .read = procfs_pid_memstat_read,
};

const file_ops_t procfs_pid_exe_ops = {
    .can_read = procfs_pid_exe_can_read,
    .read = procfs_pid_exe_read,
};

static const procfs_files_t static_procfs_files[] = {
    { .name = "memstat", .mode = S_IFREG | 0444, .ops = &procfs_pid_memstat_ops },
    { .name = "stat", .mode = S_IFREG | 0444, .ops = &procfs_pid_stat_ops },
    { .name = "exe", .mode = S_IFREG | 0444, .ops = &procfs_pid_exe_ops },
};
#define PROCFS_STATIC_FILES_COUNT_AT_LEVEL (sizeof(static_procfs_files) / sizeof(procfs_files_t))

/**
 * HELPERS
 */

pid_t procfs_pid_get_pid_from_inode_index(ino_t inode_index)
{
    return (inode_index >> 18) & 0b1111111111;
}

static ino_t procfs_pid_sfiles_get_inode_index(dentry_t* dir, int fileid)
{
    uint32_t owner_pid = procfs_root_get_pid_from_inode_index(dir->inode_indx);
    uint32_t body = (owner_pid << 18) | (fileid & 0x3ffff);
    return procfs_inode_get_index(PROCFS_PID_LEVEL, body);
}

/**
 * PID
 */

int procfs_pid_getdents(dentry_t* dir, void __user* buf, off_t* offset, size_t len)
{
    int already_read = 0;
    dirent_t tmp;

    for (int i = 0; i < PROCFS_STATIC_FILES_COUNT_AT_LEVEL; i++) {
        if (*offset <= i) {
            uint32_t inode_index = procfs_pid_sfiles_get_inode_index(dir, i);
            ssize_t read = vfs_helper_write_dirent((dirent_t __user*)(buf + already_read), len, inode_index, static_procfs_files[i].name);
            if (read <= 0) {
                if (!already_read) {
                    return -EINVAL;
                }
                return already_read;
            }
            already_read += read;
            len -= read;
            (*offset)++;
        }
    }

    return already_read;
}

int procfs_pid_lookup(const path_t* path, const char* name, size_t len, path_t* result)
{
    dentry_t* dir = path->dentry;
    procfs_inode_t* procfs_inode = (procfs_inode_t*)dir->inode;

    if (len == 1) {
        if (name[0] == '.') {
            result->dentry = dentry_duplicate(dir);
            return 0;
        }
    }

    if (len == 2) {
        if (name[0] == '.' && name[1] == '.') {
            result->dentry = dentry_duplicate(dir->parent);
            return 0;
        }
    }

    for (int i = 0; i < PROCFS_STATIC_FILES_COUNT_AT_LEVEL; i++) {
        uint32_t child_name_len = strlen(static_procfs_files[i].name);
        if (len == child_name_len) {
            if (strncmp(name, static_procfs_files[i].name, len) == 0) {
                int newly_allocated;
                result->dentry = dentry_get_no_inode(dir->dev_indx, procfs_pid_sfiles_get_inode_index(dir, i), &newly_allocated);
                if (newly_allocated) {
                    procfs_inode_t* new_procfs_inode = (procfs_inode_t*)(result->dentry->inode);
                    new_procfs_inode->mode = static_procfs_files[i].mode;
                    new_procfs_inode->ops = static_procfs_files[i].ops;
                }
                return 0;
            }
        }
    }

    return -ENOENT;
}

/**
 * FILES
 */

static bool procfs_pid_memstat_can_read(file_t* file, size_t start)
{
    return true;
}

static int procfs_pid_memstat_read(file_t* file, void __user* buf, size_t start, size_t len)
{
    if (start == 12) {
        return 0;
    }

    if (len < 12) {
        return -EFAULT;
    }
    umem_copy_to_user(buf, "reading mem", 12);
    return 12;
}

static bool procfs_pid_exe_can_read(file_t* file, size_t start)
{
    return true;
}

static int procfs_pid_exe_read(file_t* file, void __user* buf, size_t start, size_t len)
{
    dentry_t* dentry = file_dentry_assert(file);

    int pid = procfs_pid_get_pid_from_inode_index(dentry->inode_indx);
    thread_t* th = thread_by_pid(pid);
    if (!th) {
        return -EFAULT;
    }

    int req_len = vfs_get_absolute_path(&th->process->proc_file->path, NULL, 0);
    if (len < req_len) {
        return -EFAULT;
    }

    if (start == req_len - 1) {
        return 0;
    }

    // Currently we create a buffer from kernel to implement "copy to user".
    // It might be better to mark buf in vfs_get_absolute_path as __user.
    char* kbuf = (char*)kmalloc(req_len + 1);
    req_len = vfs_get_absolute_path(&th->process->proc_file->path, (char*)kbuf, len);
    umem_copy_to_user(buf, kbuf, req_len);
    return req_len - 1;
}