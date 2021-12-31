/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_FS_PROCFS_PROCFS_H
#define _KERNEL_FS_PROCFS_PROCFS_H

#include <fs/vfs.h>
#include <libkern/c_attrs.h>
#include <libkern/types.h>

struct procfs_files {
    char* name;
    mode_t mode;
    const file_ops_t* ops;
    uint32_t (*inode_index)(int);
};
typedef struct procfs_files procfs_files_t;

#define PROCFS_INODE_LEN (sizeof(struct procfs_inode))
struct PACKED procfs_inode {
    mode_t mode;
    uint16_t uid;
    uint32_t size;
    uint32_t atime;
    uint32_t ctime;
    uint32_t mtime;
    uint32_t dtime;
    uint16_t gid;
    uint16_t links_count;
    uint32_t blocks;
    uint32_t flags;
    uint32_t osd1;

    /* NOTE: Instead of blocks here, we store procfs required things */
    uint32_t index;
    const struct file_ops* ops;
    uint8_t padding[52];
    /* Block hack ends here */

    uint32_t generation;
    uint32_t file_acl;
    uint32_t dir_acl;
    uint32_t faddr;
    uint32_t osd2[3];
};
typedef struct procfs_inode procfs_inode_t;

void procfs_install();
int procfs_mount();

uint32_t procfs_root_get_pid_from_inode_index(uint32_t inode_index);

static inline uint32_t procfs_get_inode_index(const uint32_t level, uint32_t main)
{
    return (level << 28) | (main & 0x0fffffff);
}

#endif // _KERNEL_FS_PROCFS_PROCFS_H