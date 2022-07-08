/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_FS_VFS_H
#define _KERNEL_FS_VFS_H

#include <algo/sync_ringbuffer.h>
#include <drivers/driver_manager.h>
#include <fs/ext2/ext2.h>
#include <libkern/lock.h>
#include <libkern/syscall_structs.h>

#define DENTRY_WAS_IN_CACHE 0
#define DENTRY_NEWLY_ALLOCATED 1

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
    size_t blksize;
} fsdata_t;

typedef struct {
    device_t* dev;

    int fs;
    spinlock_t fslock;
} vfs_device_t;

struct dirent {
    ino_t inode;
    uint16_t rec_len;
    uint8_t name_len;
    uint8_t file_type;
    char* name;
};
typedef struct dirent dirent_t;

// Dentry Flags
#define DENTRY_DIRTY 0x1
#define DENTRY_MOUNTPOINT 0x2
#define DENTRY_MOUNTED 0x4
#define DENTRY_INODE_TO_BE_DELETED 0x8
#define DENTRY_PRIVATE 0x10 /* This dentry can't be opened so can't be copied */
#define DENTRY_CUSTOM 0x20 /* Such dentries won't be process in dentry.c file */
typedef uint32_t dentry_flag_t;

struct dentry {
    size_t d_count;
    dentry_flag_t flags;
    ino_t inode_indx;
    inode_t* inode;

    // Lock controls all dentry fields, (including inode, since it could be hold only by one dentry).
    spinlock_t lock;
    fsdata_t fsdata;
    struct fs_ops* ops;
    dev_t dev_indx;
    vfs_device_t* dev;

    char* filename;
    struct dentry* parent;
    struct dentry* mountpoint;
    struct dentry* mounted_dentry;

    struct socket* sock;
};
typedef struct dentry dentry_t;

struct path {
    dentry_t* dentry;
};
typedef struct path path_t;

struct file;
struct file_descriptor;
struct file_ops {
    bool (*can_read)(struct file* file, size_t start);
    bool (*can_write)(struct file* file, size_t start);
    int (*read)(struct file* file, void __user* buf, size_t start, size_t len);
    int (*write)(struct file* file, void __user* buf, size_t start, size_t len);
    int (*truncate)(struct file* file, size_t len);
    int (*open)(const path_t* path, struct file_descriptor* fd, uint32_t flags);
    int (*create)(const path_t* path, const char* name, size_t len, mode_t mode, uid_t uid, gid_t gid);
    int (*unlink)(const path_t* path);
    int (*getdents)(dentry_t* dir, void __user* buf, off_t* offset, size_t len);
    int (*lookup)(const path_t* path, const char* name, size_t len, path_t* result_path);
    int (*mkdir)(const path_t* path, const char* name, size_t len, mode_t mode, uid_t uid, gid_t gid);
    int (*rmdir)(const path_t* path);
    int (*ioctl)(struct file* file, uintptr_t cmd, uintptr_t arg);
    int (*fstat)(struct file* file, stat_t* stat);
    int (*fchmod)(struct file* file, mode_t mode);
    struct memzone* (*mmap)(struct file* file, mmap_params_t* params);
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
    int (*recognize)(vfs_device_t* dev);
    int (*prepare_fs)(vfs_device_t* dev);
    int (*eject_device)(vfs_device_t* dev);

    file_ops_t file;
    dentry_ops_t dentry;
};
typedef struct fs_ops fs_ops_t;

struct fs_desc {
    driver_t* driver;
    fs_ops_t* ops;
};
typedef struct fs_desc fs_desc_t;

typedef uint32_t file_type_t;
enum FTYPES {
    FTYPE_FILE,
    FTYPE_SOCKET,
};

struct file {
    size_t count;
    file_type_t type;
    union {
        dentry_t* dentry; // type == FTYPE_FILE
        struct socket* socket; // type == FTYPE_SOCKET
    };
    uint32_t flags;
    path_t path;
    file_ops_t* ops;

    // Used by socket to keep data.
    void* auxdata;

    // Protects flags.
    spinlock_t lock;
};
typedef struct file file_t;

struct file_descriptor {
    file_t* file;
    off_t offset;
    int flags;
};
typedef struct file_descriptor file_descriptor_t;

struct socket {
    size_t d_count;
    int domain;
    int type;
    int protocol;
    mode_t mode;
    sync_ringbuffer_t buffer;
    file_t* bind_file;
    spinlock_t lock;
};
typedef struct socket socket_t;

/**
 * DENTRIES
 */

void kdentryflusherd();

void dentry_set_parent(dentry_t* to, dentry_t* parent);
void dentry_set_filename(dentry_t* to, char* filename);
dentry_t* dentry_get(dev_t dev_indx, ino_t inode_indx);
dentry_t* dentry_get_no_inode(dev_t dev_indx, ino_t inode_indx, int* newly_allocated);
dentry_t* dentry_get_parent(dentry_t* dentry);
dentry_t* dentry_duplicate(dentry_t* dentry);
void dentry_put(dentry_t* dentry);
void dentry_force_put(dentry_t* dentry);
void dentry_put_all_dentries_of_dev(dev_t dev_indx);
int dentry_flush(dentry_t* dentry);
void dentry_set_inode(dentry_t* dentry, inode_t* inode);
void dentry_set_flag(dentry_t* dentry, dentry_flag_t flag);
bool dentry_test_flag(dentry_t* dentry, dentry_flag_t flag);
void dentry_rem_flag(dentry_t* dentry, dentry_flag_t flag);
void dentry_inode_set_flag(dentry_t* dentry, mode_t mode);
bool dentry_test_mode(dentry_t* dentry, mode_t mode);
void dentry_inode_rem_flag(dentry_t* dentry, mode_t mode);

void dentry_put_locked(dentry_t* dentry);
void dentry_set_flag_locked(dentry_t* dentry, dentry_flag_t flag);
bool dentry_test_flag_locked(dentry_t* dentry, dentry_flag_t flag);
void dentry_rem_flag_locked(dentry_t* dentry, dentry_flag_t flag);
bool dentry_test_mode_locked(dentry_t* dentry, mode_t mode);

size_t dentry_stat_cached_count();

/**
 * VFS HELPERS
 */

ssize_t vfs_helper_write_dirent(dirent_t __user* buf, size_t buf_len, ino_t inode_index, const char* name);
char* vfs_helper_split_path_with_name(char* name, size_t len);
void vfs_helper_restore_full_path_after_split(char* path, char* name);

/**
 * FILE HELPERS
 */

static inline dentry_t* file_dentry(file_t* file) { return file->type == FTYPE_FILE ? file->dentry : NULL; }
static inline socket_t* file_socket(file_t* file) { return file->type == FTYPE_SOCKET ? file->socket : NULL; }
static inline dentry_t* file_dentry_assert(file_t* file)
{
    ASSERT(file->type == FTYPE_FILE);
    return file->dentry;
}
static inline socket_t* file_socket_assert(file_t* file)
{
    ASSERT(file->type == FTYPE_SOCKET);
    return file->socket;
}
file_t* file_init_pseudo_dentry(dentry_t* pseudo_dentry);
file_t* file_init_socket(socket_t* socket, file_ops_t* ops);
file_t* file_init_path(const path_t* path);

file_t* file_duplicate(file_t* file);
file_t* file_duplicate_locked(file_t* file);
void file_put(file_t* file);

static inline bool path_is_valid(const path_t* path) { return path && path->dentry; }
path_t path_duplicate(const path_t* path);
void path_put(path_t* path);
inline path_t vfs_empty_path()
{
    path_t a = { .dentry = NULL };
    return a;
}

/**
 * VFS APIS
 */

void vfs_install();
int vfs_add_dev(device_t* dev);
int vfs_add_dev_with_fs(device_t* dev, int fs_id);
int vfs_add_fs(driver_t* fs);
int vfs_get_fs_id(const char* name);
void vfs_eject_device(device_t* t_new_dev);

int vfs_resolve_path(const char* path, path_t* result);
int vfs_resolve_path_start_from(const path_t* vfspath, const char* path, path_t* result);

int vfs_create(const path_t* path, const char* name, size_t len, mode_t mode, uid_t uid, gid_t gid);
int vfs_unlink(const path_t* path);
int vfs_lookup(const path_t* path, const char* name, size_t len, path_t* result);
int vfs_open(const path_t* path, file_descriptor_t* fd, int flags);
int vfs_close(file_descriptor_t* fd);
bool vfs_can_read(file_descriptor_t* fd);
bool vfs_can_write(file_descriptor_t* fd);
int vfs_read(file_descriptor_t* fd, void __user* buf, size_t len);
int vfs_write(file_descriptor_t* fd, void __user* buf, size_t len);
int vfs_mkdir(const path_t* path, const char* name, size_t len, mode_t mode, uid_t uid, gid_t gid);
int vfs_rmdir(const path_t* path);
int vfs_getdents(file_descriptor_t* dir_fd, void __user* buf, size_t len);
int vfs_fstat(file_descriptor_t* fd, stat_t* stat);
int vfs_chmod(const path_t* path, mode_t mode);
int vfs_fchmod(file_descriptor_t* fd, mode_t mode);

int vfs_get_absolute_path(const path_t* path, char* buf, int len);

int vfs_mount(path_t* mount_path, device_t* dev, uint32_t fs_indx);
int vfs_umount(dentry_t* mountpoint);

struct proc;
struct memzone* vfs_mmap(file_descriptor_t* fd, mmap_params_t* params);
int vfs_munmap(struct proc* p, struct memzone*);

struct thread;
int vfs_check_open_perms(const path_t* path, int flags);
int vfs_perm_to_read(dentry_t* dentry, struct thread* t);
int vfs_perm_to_write(dentry_t* dentry, struct thread* t);
int vfs_perm_to_execute(dentry_t* dentry, struct thread* t);

#endif // _KERNEL_FS_VFS_H
