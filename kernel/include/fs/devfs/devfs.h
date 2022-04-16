/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_FS_DEVFS_DEVFS_H
#define _KERNEL_FS_DEVFS_DEVFS_H

#include <fs/vfs.h>
#include <libkern/c_attrs.h>
#include <libkern/types.h>

#define DEVFS_INODE_LEN (sizeof(struct devfs_inode))
struct PACKED devfs_inode {
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

    /* NOTE: Instead of blocks here, we store devfs required things */
    uint32_t index;
    uint32_t dev_id;
    char* name;
    struct file_ops* handlers;
    struct devfs_inode* parent;
    struct devfs_inode* prev;
    struct devfs_inode* next;
    struct devfs_inode* first;
    struct devfs_inode* last;
    uint8_t padding[24];
    /* Block hack ends here */

    uint32_t generation;
    uint32_t file_acl;
    uint32_t dir_acl;
    uint32_t faddr;
    uint32_t osd2[3];
};
typedef struct devfs_inode devfs_inode_t;

void devfs_install();
int devfs_mount();

devfs_inode_t* devfs_mkdir(const path_t* vfspath, const char* name, uint32_t len);
devfs_inode_t* devfs_register(const path_t* vfspath, uint32_t devid, const char* name, uint32_t len, mode_t mode, const file_ops_t* handlers);

#endif /* _KERNEL_FS_DEVFS_DEVFS_H */