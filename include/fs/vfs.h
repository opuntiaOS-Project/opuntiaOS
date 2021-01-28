/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#ifndef __oneOS__FS__VFS_H
#define __oneOS__FS__VFS_H

#include <algo/ringbuffer.h>
#include <drivers/driver_manager.h>
#include <fs/ext2/ext2.h>
#include <syscall_structs.h>

#define VFS_MAX_FS_COUNT 5
#define VFS_MAX_DEV_COUNT 5
#define VFS_MAX_FILENAME 16
#define VFS_MAX_FILENAME_EXT 4
#define VFS_ATTR_NOTFILE 0xff
#define VFS_USE_STD_MMAP 0xffffffff /* If custom mmap impl isn't support for such a file, you can return the flag and std impl will be used */

typedef struct {
    uint32_t count;
    group_desc_t* table;
} groups_info_t;

typedef struct {
    superblock_t* sb;
    groups_info_t* gt;
} fsdata_t;

typedef struct {
    int fs;
    device_t* dev;
} vfs_device_t;

struct dirent {
    uint32_t inode;
    uint16_t rec_len;
    uint8_t name_len;
    uint8_t file_type;
    char* name;
};
typedef struct dirent dirent_t;

#define DENTRY_DIRTY 0x1
#define DENTRY_MOUNTPOINT 0x2
#define DENTRY_MOUNTED 0x4
#define DENTRY_INODE_TO_BE_DELETED 0x8
#define DENTRY_PRIVATE 0x10 /* This dentry can't be opened so can't be copied */
#define DENTRY_CUSTOM 0x20 /* Such dentries won't be process in dentry.c file */
struct dentry {
    uint32_t d_count;
    uint32_t flags;
    uint32_t inode_indx;
    inode_t* inode;
    fsdata_t fsdata;
    struct fs_ops* ops;
    uint32_t dev_indx;
    vfs_device_t* dev;

    uint32_t parent_inode_indx;
    uint32_t parent_dev_indx;

    struct dentry* mountpoint;
    struct dentry* mounted_dentry;

    struct socket* sock;
};
typedef struct dentry dentry_t;

struct dentry_cache_list {
    struct dentry_cache_list* prev;
    struct dentry_cache_list* next;
    dentry_t* data;
    uint32_t len;
};
typedef struct dentry_cache_list dentry_cache_list_t;

struct file_descriptor;
struct file_ops {
    bool (*can_read)(dentry_t*, uint32_t start);
    bool (*can_write)(dentry_t*, uint32_t start);
    int (*read)(dentry_t*, uint8_t*, uint32_t, uint32_t);
    int (*write)(dentry_t*, uint8_t*, uint32_t, uint32_t);
    int (*open)(dentry_t* dentry, struct file_descriptor* fd, uint32_t flags);
    int (*truncate)(dentry_t*, uint32_t);
    int (*create)(dentry_t* dentry, const char* name, uint32_t len, mode_t mode);
    int (*unlink)(dentry_t* dentry);
    int (*getdents)(dentry_t* dir, uint8_t* buf, uint32_t* offset, uint32_t len);
    int (*lookup)(dentry_t* dentry, const char* name, uint32_t len, uint32_t* res_inode_indx);
    int (*mkdir)(dentry_t* dir, const char* name, uint32_t len, mode_t mode);
    int (*rmdir)(dentry_t* dir);
    int (*ioctl)(dentry_t* dentry, uint32_t cmd, uint32_t arg);
    struct proc_zone* (*mmap)(dentry_t* dentry, mmap_params_t* params);
};
typedef struct file_ops file_ops_t;

struct dentry_ops {
    int (*read_inode)(dentry_t* dentry);
    int (*write_inode)(dentry_t* dentry);
    int (*free_inode)(dentry_t* dentry);
    fsdata_t (*get_fsdata)(dentry_t* dentry);
};
typedef struct dentry_ops dentry_ops_t;

struct fs_ops {
    void* recognize;
    void* prepare_fs;
    void* eject_device;

    file_ops_t file;
    dentry_ops_t dentry;
};
typedef struct fs_ops fs_ops_t;

enum FD_TYPE {
    FD_TYPE_FILE,
    FD_TYPE_SOCKET,
};

struct file_descriptor {
    uint32_t type;
    union {
        dentry_t* dentry; // type == FD_TYPE_FILE
        struct socket* sock_entry; // type == FD_TYPE_SOCKET
    };
    uint32_t offset;
    uint32_t flags;
    uint32_t mapped_times;
    file_ops_t* ops;
};
typedef struct file_descriptor file_descriptor_t;

struct socket {
    uint32_t d_count;
    int domain;
    int type;
    int protocol;
    ringbuffer_t buffer;
    file_descriptor_t bind_file;
};
typedef struct socket socket_t;

/**
 * DENTRIES
 */

void dentry_flusher();

void dentry_set_parent(dentry_t* to, dentry_t* parent);
dentry_t* dentry_get(uint32_t dev_indx, uint32_t inode_indx);
dentry_t* dentry_get_parent(dentry_t* dentry);
void dentry_add(uint32_t dev_indx, uint32_t inode_indx, dentry_t res);
dentry_t* dentry_duplicate(dentry_t* dentry);
void dentry_put(dentry_t* dentry);
void dentry_force_put(dentry_t* dentry);
void dentry_put_all_dentries_of_dev(uint32_t dev_indx);
void dentry_set_flag(dentry_t* dentry, uint32_t flag);
bool dentry_test_flag(dentry_t* dentry, uint32_t flag);
void dentry_rem_flag(dentry_t* dentry, uint32_t flag);
void dentry_inode_set_flag(dentry_t* dentry, mode_t mode);
bool dentry_inode_test_flag(dentry_t* dentry, mode_t mode);
void dentry_inode_rem_flag(dentry_t* dentry, mode_t mode);

uint32_t dentry_stat_cached_count();

/**
 * VFS APIS
 */

void vfs_install();
int vfs_add_dev(device_t* dev);
int vfs_add_dev_with_fs(device_t* dev, int fs_id);
void vfs_add_fs(driver_t* t_new_fs);
void vfs_eject_device(device_t* t_new_dev);

int vfs_resolve_path(const char* path, dentry_t** result);
int vfs_resolve_path_start_from(dentry_t* dentry, const char* path, dentry_t** result);

int vfs_create(dentry_t* dir, const char* name, uint32_t len, mode_t mode);
int vfs_unlink(dentry_t* file);
int vfs_lookup(dentry_t* dir, const char* name, uint32_t len, dentry_t** result);
int vfs_open(dentry_t* file, file_descriptor_t* fd, uint32_t flags);
int vfs_close(file_descriptor_t* fd);
bool vfs_can_read(file_descriptor_t* fd);
bool vfs_can_write(file_descriptor_t* fd);
int vfs_read(file_descriptor_t* fd, void* buf, uint32_t len);
int vfs_write(file_descriptor_t* fd, void* buf, uint32_t len);
int vfs_mkdir(dentry_t* dir, const char* name, uint32_t len, mode_t mode);
int vfs_rmdir(dentry_t* dir);
int vfs_getdents(file_descriptor_t* dir_fd, uint8_t* buf, uint32_t len);

int vfs_mount(dentry_t* mountpoint, device_t* dev, uint32_t fs_indx);
int vfs_umount(dentry_t* mountpoint);

struct proc_zone* vfs_mmap(file_descriptor_t* fd, mmap_params_t* params);

#endif // __oneOS__FS__VFS_H
