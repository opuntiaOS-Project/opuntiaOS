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
#include <tasking/tasking.h>

/**
 * inode: xxxxBBBBBBBBBBBBBBBBBBBBBBBBBBBB
 * x - level bits
 * B - bits of files at this level
 */
#define PROCFS_SYS_LEVEL 3

static int _sysctl_enable = 1;

/* PID */
int procfs_sys_getdents(dentry_t* dir, uint8_t* buf, off_t* offset, size_t len);
int procfs_sys_lookup(dentry_t* dir, const char* name, size_t len, dentry_t** result);

/* FILES */
static bool procfs_sys_doint_can_read(dentry_t* dentry, uint32_t start);
static int procfs_sys_doint_read(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len);

/**
 * DATA
 */

const file_ops_t procfs_sys_ops = {
    .getdents = procfs_sys_getdents,
    .lookup = procfs_sys_lookup,
};

const file_ops_t procfs_sys_doint_ops = {
    .can_read = procfs_sys_doint_can_read,
    .read = procfs_sys_doint_read,
};

static const procfs_files_t static_procfs_files[] = {
    { .name = "sysctl_enable", .mode = 0444, .ops = &procfs_sys_doint_ops, .data = &_sysctl_enable },
};
#define PROCFS_STATIC_FILES_COUNT_AT_LEVEL (sizeof(static_procfs_files) / sizeof(procfs_files_t))

/**
 * HELPERS
 */

static const procfs_files_t* procfs_sys_get_sfile(dentry_t* file)
{
    uint32_t body = procfs_inode_get_body(file->inode_indx);
    if (body >= PROCFS_STATIC_FILES_COUNT_AT_LEVEL) {
        return NULL;
    }
    return &static_procfs_files[body];
}

static uint32_t procfs_sys_sfiles_get_inode_index(dentry_t* dir, int fileid)
{
    return procfs_inode_get_index(PROCFS_SYS_LEVEL, fileid);
}

/**
 * PID
 */

int procfs_sys_getdents(dentry_t* dir, uint8_t* buf, off_t* offset, size_t len)
{
    int already_read = 0;
    dirent_t tmp;

    for (int i = 0; i < PROCFS_STATIC_FILES_COUNT_AT_LEVEL; i++) {
        if (*offset <= i) {
            uint32_t inode_index = procfs_sys_sfiles_get_inode_index(dir, i);
            ssize_t read = vfs_helper_write_dirent((dirent_t*)(buf + already_read), len, inode_index, static_procfs_files[i].name);
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

int procfs_sys_lookup(dentry_t* dir, const char* name, size_t len, dentry_t** result)
{
    procfs_inode_t* procfs_inode = (procfs_inode_t*)dir->inode;

    if (len == 1) {
        if (name[0] == '.') {
            *result = dentry_duplicate(dir);
            return 0;
        }
    }

    if (len == 2) {
        if (name[0] == '.' && name[1] == '.') {
            *result = dentry_duplicate(dir->parent);
            return 0;
        }
    }

    for (int i = 0; i < PROCFS_STATIC_FILES_COUNT_AT_LEVEL; i++) {
        uint32_t child_name_len = strlen(static_procfs_files[i].name);
        if (len == child_name_len) {
            if (strncmp(name, static_procfs_files[i].name, len) == 0) {
                int newly_allocated;
                *result = dentry_get_no_inode(dir->dev_indx, procfs_sys_sfiles_get_inode_index(dir, i), &newly_allocated);
                if (newly_allocated) {
                    procfs_inode_t* new_procfs_inode = (procfs_inode_t*)((*result)->inode);
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

static bool procfs_sys_doint_can_read(dentry_t* dentry, uint32_t start)
{
    return true;
}

static int procfs_sys_doint_read(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len)
{
    int* data = procfs_sys_get_sfile(dentry)->data;

    char res[16];
    snprintf(res, 16, "%d", *data);
    size_t size = strlen(res);

    if (start == size) {
        return 0;
    }

    if (len < size) {
        return -EFAULT;
    }

    memcpy(buf, res, size);
    return size;
}