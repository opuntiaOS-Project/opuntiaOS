/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#ifndef __oneOS__FS__VFS_H
#define __oneOS__FS__VFS_H

#include <drivers/driver_manager.h>
#include <fs/ext2/ext2.h>

#define VFS_MAX_FS_COUNT 5
#define VFS_MAX_DEV_COUNT 5
#define VFS_MAX_FILENAME 16
#define VFS_MAX_FILENAME_EXT 4
#define VFS_ATTR_NOTFILE 0xff

typedef struct {
    superblock_t* sb;
    group_desc_t* gt;
} fsdata_t;

typedef struct {
    int8_t fs;
    device_t *dev;
} vfs_device_t;

#define DENTRY_DIRTY 0x1
#define DENTRY_MOUNTPOINT 0x2
struct dentry{
    uint32_t d_count;
    uint32_t flags;
    uint32_t inode_indx;
    inode_t* inode;
    fsdata_t fsdata;
    struct fs_ops* ops;
    uint32_t dev_indx;
    vfs_device_t* dev;
};
typedef struct dentry dentry_t;

struct fs_ops {
    void* recognize;

    void* create_dir;
    void* lookup_dir;
    void* remove_dir;

    void* write_file;
    void* read_file;
    void* remove_file;

    void* eject_device;

    void* open;
    void* read;
    void* write;
    int (*mkdir)(dentry_t* dir, const char* name, uint32_t len, uint16_t mode);
    int (*read_inode)(dentry_t* dentry);
    int (*write_inode)(dentry_t* dentry);
    fsdata_t (*get_fsdata)(dentry_t* dentry);
    int (*lookup)(dentry_t* dentry, const char* name, uint32_t len, uint32_t* res_inode_indx);
};
typedef struct fs_ops fs_ops_t;

struct dentry_cache_list {
    struct dentry_cache_list* prev;
    struct dentry_cache_list* next;
    dentry_t* data;
    uint32_t len;
};
typedef struct dentry_cache_list dentry_cache_list_t;

typedef struct {
    dentry_t* dentry;
    fs_ops_t* ops;
} file_descriptor_t;


/**
 * DENTRIES
 */

dentry_t* dentry_get(uint32_t dev_indx, uint32_t inode_indx);
void dentry_add(uint32_t dev_indx, uint32_t inode_indx, dentry_t res);
dentry_t* dentry_duplicate(dentry_t* dentry);
void dentry_put(dentry_t* dentry);
void dentry_put_all_dentries_of_dev(uint32_t dev_indx);
void dentry_set_flag(dentry_t* dentry, uint32_t flag);
bool dentry_test_flag(dentry_t* dentry, uint32_t flag);
void dentry_rem_flag(dentry_t* dentry, uint32_t flag);

/**
 * VFS APIS
 */

void vfs_install();
void vfs_add_device(device_t* t_new_dev);
void vfs_add_fs(driver_t* t_new_fs);
void vfs_eject_device(device_t* t_new_dev);

void open();
void close();

/* NEW APIS */

int vfs_resolve_path(const char* path, dentry_t** result);
int vfs_resolve_path_start_from(dentry_t* dentry, const char* path, dentry_t** result);

int vfs_lookup(dentry_t* dir, const char* name, uint32_t len, dentry_t** result);
int vfs_open(dentry_t* file, file_descriptor_t* fd);
int vfs_read(file_descriptor_t* fd, uint8_t* buf, uint32_t start, uint32_t len);
int vfs_write(file_descriptor_t* fd, uint8_t* buf, uint32_t start, uint32_t len);
int vfs_mkdir(dentry_t* dir, const char* name, uint32_t len, uint16_t mode);

#endif // __oneOS__FS__VFS_H
