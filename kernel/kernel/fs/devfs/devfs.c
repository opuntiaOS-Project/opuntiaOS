/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <algo/dynamic_array.h>
#include <drivers/driver_manager.h>
#include <fs/devfs/devfs.h>
#include <fs/vfs.h>
#include <libkern/bits/errno.h>
#include <libkern/libkern.h>
#include <libkern/lock.h>
#include <libkern/log.h>
#include <mem/kmalloc.h>
#include <tasking/proc.h>

#define DEVFS_ZONE_SIZE 32 * KB

static dynamic_array_t entry_zones; /* Just store pointers to free zones one time. */
static void* next_space_to_put_entry;
static size_t free_space_in_last_entry_zone;

static dynamic_array_t name_zones; /* Just store pointers to free zones one time. */
static void* next_space_to_put_name;
static size_t free_space_in_last_name_zone;

static dynamic_array_t ops_zones; /* Just store pointers to free zones one time. */
static void* next_space_to_put_ops;
static size_t free_space_in_last_ops_zone;

static devfs_inode_t* devfs_root;

static ino_t next_inode_index = 2;

static spinlock_t _devfs_lock;

/**
 * Zones Management
 */

static int _devfs_alloc_entry_zone()
{
    void* new_zone = kmalloc(DEVFS_ZONE_SIZE);
    if (!new_zone) {
        return ENOMEM;
    }

    dynarr_push(&entry_zones, &new_zone);
    next_space_to_put_entry = new_zone;
    free_space_in_last_entry_zone = DEVFS_ZONE_SIZE;
    return 0;
}

static int _devfs_alloc_name_zone()
{
    void* new_zone = kmalloc(DEVFS_ZONE_SIZE);
    if (!new_zone) {
        return -ENOMEM;
    }

    dynarr_push(&name_zones, &new_zone);
    next_space_to_put_name = new_zone;
    free_space_in_last_name_zone = DEVFS_ZONE_SIZE;
    return 0;
}

static int _devfs_alloc_ops_zone()
{
    void* new_zone = kmalloc(DEVFS_ZONE_SIZE);
    if (!new_zone) {
        return -ENOMEM;
    }

    dynarr_push(&ops_zones, &new_zone);
    next_space_to_put_ops = new_zone;
    free_space_in_last_ops_zone = DEVFS_ZONE_SIZE;
    return 0;
}

static devfs_inode_t* _devfs_get_entry(ino_t indx)
{
    size_t entries_per_zone = DEVFS_ZONE_SIZE / DEVFS_INODE_LEN;
    size_t zones_count = entry_zones.size;
    if (indx >= zones_count * entries_per_zone) {
        return 0;
    }

    size_t requested_zone = indx / entries_per_zone;
    size_t index_within_zone = indx % entries_per_zone;
    uintptr_t* tmp_ptr = dynarr_get(&entry_zones, requested_zone);
    devfs_inode_t* res = (devfs_inode_t*)*tmp_ptr;
    if (!res) {
        return 0;
    }
    return &res[index_within_zone];
}

/**
 * Memory Allocation
 */

static devfs_inode_t* _devfs_new_entry()
{
    if (free_space_in_last_entry_zone < sizeof(devfs_inode_t)) {
        _devfs_alloc_entry_zone();
    }

    devfs_inode_t* res = (devfs_inode_t*)next_space_to_put_entry;
    next_space_to_put_entry += sizeof(devfs_inode_t);
    free_space_in_last_entry_zone -= sizeof(devfs_inode_t);
    memset(res, 0, DEVFS_INODE_LEN);
    res->index = next_inode_index++;
    return res;
}

static char* _devfs_new_name(size_t len)
{
    len = (len + 1 + 0x3) & (size_t)(~0b11);
    if (free_space_in_last_name_zone < len) {
        _devfs_alloc_name_zone();
    }

    char* res = (char*)next_space_to_put_name;
    next_space_to_put_name += len;
    free_space_in_last_name_zone -= len;
    memset(res, 0, len);
    return res;
}

static file_ops_t* _devfs_new_ops()
{
    if (free_space_in_last_ops_zone < sizeof(file_ops_t)) {
        _devfs_alloc_ops_zone();
    }

    file_ops_t* res = (file_ops_t*)next_space_to_put_ops;
    next_space_to_put_ops += sizeof(file_ops_t);
    free_space_in_last_ops_zone -= sizeof(file_ops_t);
    return res;
}

static inline devfs_inode_t* _devfs_get_devfs_inode(ino_t inode_indx)
{
    /*  Since in vfs, we use a rule that 2nd inode is a root inode, so, let's
        follow this rule here. But in our storage elements are indexed from 0,
        so we need put up with this. */
    return _devfs_get_entry(inode_indx - 2);
}

/**
 * Inode Tools
 */

static int _devfs_add_to_list(devfs_inode_t* parent, devfs_inode_t* new_entry)
{
    if (!parent || !new_entry) {
        return -EINVAL;
    }

    if (!parent->last) {
        parent->first = new_entry;
        parent->last = new_entry;
        new_entry->prev = 0;
        new_entry->next = 0;
    } else {
        parent->last->next = new_entry;
        new_entry->prev = parent->last;
        parent->last = new_entry;
        new_entry->next = 0;
    }

    new_entry->parent = parent;

    return 0;
}

static devfs_inode_t* _devfs_alloc_entry(devfs_inode_t* parent)
{
    devfs_inode_t* new_entry = _devfs_new_entry();
    if (!new_entry) {
        return 0;
    }
    if (_devfs_add_to_list(parent, new_entry) < 0) {
        /* Bringing back allocation */
        next_space_to_put_entry -= sizeof(devfs_inode_t);
        free_space_in_last_entry_zone += sizeof(devfs_inode_t);
        return 0;
    }
    return new_entry;
}

static int _devfs_set_name(devfs_inode_t* entry, const char* name, size_t len)
{
    if (!entry || !name) {
        return -EINVAL;
    }
    if (len > 255) {
        return -2;
    }

    char* name_space = _devfs_new_name(len);
    if (!name_space) {
        return -ENOMEM;
    }

    memcpy((void*)name_space, (void*)name, len);
    entry->name = name_space;
    return 0;
}

static int _devfs_set_handlers(devfs_inode_t* entry, const file_ops_t* ops)
{
    if (!entry) {
        return -EINVAL;
    }

    file_ops_t* ops_space = _devfs_new_ops();
    if (!ops_space) {
        return -ENOMEM;
    }

    memcpy(ops_space, ops, sizeof(file_ops_t));
    entry->handlers = ops_space;
    return 0;
}

/**
 * FS Tools
 */

static int _devfs_setup_root()
{
    devfs_root = _devfs_new_entry();
    devfs_root->index = 2;
    devfs_root->mode = S_IFDIR;
    return 0;
}

/**
 * VFS Api
 */

int devfs_prepare_fs(vfs_device_t* vdev)
{
    dynarr_init(void*, &name_zones);
    dynarr_init(void*, &entry_zones);
    dynarr_init(void*, &ops_zones);

    _devfs_setup_root();
    return 0;
}

int devfs_read_inode(dentry_t* dentry)
{
    spinlock_acquire(&_devfs_lock);
    /*  We currently have a uniqueue structure of inode for devfs. So we need to be
        confident using inode in vfs, since inode and devfs_inode are not similar. */
    devfs_inode_t* devfs_inode = _devfs_get_devfs_inode(dentry->inode_indx);
    if (!devfs_inode) {
        spinlock_release(&_devfs_lock);
        return -EFAULT;
    }
    memcpy((void*)dentry->inode, (void*)devfs_inode, DEVFS_INODE_LEN);
    spinlock_release(&_devfs_lock);
    return 0;
}

int devfs_write_inode(dentry_t* dentry)
{
    spinlock_acquire(&_devfs_lock);
    devfs_inode_t* devfs_inode = _devfs_get_devfs_inode(dentry->inode_indx);
    if (!devfs_inode) {
        spinlock_release(&_devfs_lock);
        return -EFAULT;
    }
    memcpy((void*)devfs_inode, (void*)dentry->inode, DEVFS_INODE_LEN);
    spinlock_release(&_devfs_lock);
    return 0;
}

int devfs_free_inode(dentry_t* dentry)
{
    return 0;
}

int devfs_getdents(const path_t* path, void __user* buf, off_t* offset, size_t len)
{
    dentry_t* dir = path->dentry;
    devfs_inode_t* devfs_inode = (devfs_inode_t*)dir->inode;

    /* Currently we leave, dir when offset is the max number. */
    if (*offset == 0xffffffff) {
        return 0;
    }

    int already_read = 0;
    dirent_t tmp;
    tmp.name = 0;

    /* Return . */
    if (*offset == 0) {
        ssize_t read = vfs_helper_write_dirent((dirent_t __user*)(buf + already_read), len, devfs_inode->index, ".");
        if (read <= 0) {
            if (!already_read) {
                return -EINVAL;
            }
            return already_read;
        }
        already_read += read;
        len -= read;
        *offset = 1;
    }

    /* Return .. */
    if (*offset == 1) {
        ino_t inode_index = devfs_inode->index;
        if (devfs_inode->parent) {
            inode_index = devfs_inode->parent->index;
        }

        ssize_t read = vfs_helper_write_dirent((dirent_t __user*)(buf + already_read), len, inode_index, "..");
        if (read <= 0) {
            if (!already_read) {
                return -EINVAL;
            }
            return already_read;
        }
        already_read += read;
        len -= read;
        *offset = 2;
    }

    /* Scanining dir from the start */
    spinlock_acquire(&_devfs_lock);
    if (*offset == 2) {
        if (!devfs_inode->first) {
            spinlock_release(&_devfs_lock);
            return 0;
        }
        *offset = (uintptr_t)devfs_inode->first;
    }

    while (*offset != 0xffffffff) {
        devfs_inode_t* child_devfs_inode = (devfs_inode_t*)*offset;
        ssize_t read = vfs_helper_write_dirent((dirent_t __user*)(buf + already_read), len, child_devfs_inode->index, child_devfs_inode->name);
        if (read <= 0) {
            spinlock_release(&_devfs_lock);
            if (!already_read) {
                return -EINVAL;
            }
            return already_read;
        }
        already_read += read;
        len -= read;
        if (child_devfs_inode->next) {
            *offset = (uintptr_t)child_devfs_inode->next;
        } else {
            *offset = 0xffffffff;
        }
    }

    spinlock_release(&_devfs_lock);
    return already_read;
}

int devfs_lookup(const path_t* path, const char* name, size_t len, path_t* result)
{
    dentry_t* dir = path->dentry;
    devfs_inode_t* devfs_inode = (devfs_inode_t*)dir->inode;

    if (len == 1) {
        if (name[0] == '.') {
            result->dentry = dentry_get(dir->dev_indx, devfs_inode->index);
            return 0;
        }
    }

    if (len == 2) {
        if (name[0] == '.' && name[1] == '.') {
            result->dentry = dentry_get(dir->dev_indx, devfs_inode->parent->index);
            return 0;
        }
    }

    devfs_inode_t* child = devfs_inode->first;
    while (child) {
        size_t child_name_len = strlen(child->name);
        if (len == child_name_len) {
            if (strncmp(name, child->name, len) == 0) {
                result->dentry = dentry_get(dir->dev_indx, child->index);
                return 0;
            }
        }
        child = child->next;
    }

    return -ENOENT;
}

int devfs_mkdir_dummy(const path_t* path, const char* name, size_t len, mode_t mode, uid_t uid)
{
    return -1;
}

int devfs_rmdir_dummy(const path_t* path)
{
    return -1;
}

int devfs_open(const path_t* path, file_descriptor_t* fd, uint32_t flags)
{
    devfs_inode_t* devfs_inode = (devfs_inode_t*)path->dentry->inode;
    if (devfs_inode->handlers->open) {
        return devfs_inode->handlers->open(path, fd, flags);
    }
    /*  The device doesn't have custom open, so returns ENOEXEC in this case
        according to vfs. */
    return -ENOEXEC;
}

int devfs_can_read(file_t* file, size_t start)
{
    devfs_inode_t* devfs_inode = (devfs_inode_t*)file_dentry_assert(file)->inode;
    if (devfs_inode->handlers->can_read) {
        return devfs_inode->handlers->can_read(file, start);
    }
    return true;
}

int devfs_can_write(file_t* file, size_t start)
{
    devfs_inode_t* devfs_inode = (devfs_inode_t*)file_dentry_assert(file)->inode;
    if (devfs_inode->handlers->can_write) {
        return devfs_inode->handlers->can_write(file, start);
    }
    return true;
}

int devfs_read(file_t* file, void __user* buf, size_t start, size_t len)
{
    devfs_inode_t* devfs_inode = (devfs_inode_t*)file_dentry_assert(file)->inode;
    if (devfs_inode->handlers->read) {
        return devfs_inode->handlers->read(file, buf, start, len);
    }
    return -EFAULT;
}

int devfs_write(file_t* file, void __user* buf, size_t start, size_t len)
{
    devfs_inode_t* devfs_inode = (devfs_inode_t*)file_dentry_assert(file)->inode;
    if (devfs_inode->handlers->write) {
        return devfs_inode->handlers->write(file, buf, start, len);
    }
    return -EFAULT;
}

int devfs_fstat(file_t* file, stat_t* stat)
{
    // Filling the default data, while custom fstat function of a
    // device could rewrite this.
    dentry_t* dentry = file_dentry_assert(file);
    spinlock_acquire(&dentry->lock);

    devfs_inode_t* devfs_inode = (devfs_inode_t*)(dentry->inode);
    stat->st_dev = devfs_inode->dev_id;
    stat->st_ino = devfs_inode->index;
    stat->st_mode = devfs_inode->mode;
    stat->st_size = 0;
    stat->st_uid = 0;
    stat->st_gid = 0;
    stat->st_blksize = 1;
    stat->st_nlink = 1;
    stat->st_blocks = 0;
    stat->st_atim.tv_sec = 0;
    stat->st_atim.tv_nsec = 0;
    stat->st_mtim.tv_sec = 0;
    stat->st_mtim.tv_nsec = 0;
    stat->st_ctim.tv_sec = 0;
    stat->st_ctim.tv_nsec = 0;

    spinlock_release(&dentry->lock);

    // Calling a custom fstat if present.
    if (devfs_inode->handlers->fstat) {
        return devfs_inode->handlers->fstat(file, stat);
    }
    return 0;
}

int devfs_fchmod(file_t* file, mode_t mode)
{
    devfs_inode_t* devfs_inode = (devfs_inode_t*)file_dentry_assert(file)->inode;
    if (devfs_inode->handlers->fchmod) {
        return devfs_inode->handlers->fchmod(file, mode);
    }
    return -EROFS;
}

int devfs_ioctl(file_t* file, uintptr_t cmd, uintptr_t arg)
{
    devfs_inode_t* devfs_inode = (devfs_inode_t*)file_dentry_assert(file)->inode;
    if (devfs_inode->handlers->ioctl) {
        return devfs_inode->handlers->ioctl(file, cmd, arg);
    }
    return -EFAULT;
}

memzone_t* devfs_mmap(file_t* file, mmap_params_t* params)
{
    devfs_inode_t* devfs_inode = (devfs_inode_t*)file_dentry_assert(file)->inode;
    if (devfs_inode->handlers->mmap) {
        return devfs_inode->handlers->mmap(file, params);
    }

    // If no custom impl, let's used a standard one.
    return (memzone_t*)VFS_USE_STD_MMAP;
}

/**
 * Driver install functions.
 */

driver_desc_t _devfs_driver_info()
{
    driver_desc_t fs_desc = { 0 };
    fs_desc.type = DRIVER_FILE_SYSTEM;
    fs_desc.functions[DRIVER_FILE_SYSTEM_RECOGNIZE] = NULL;
    fs_desc.functions[DRIVER_FILE_SYSTEM_PREPARE_FS] = devfs_prepare_fs;
    fs_desc.functions[DRIVER_FILE_SYSTEM_CAN_READ] = devfs_can_read;
    fs_desc.functions[DRIVER_FILE_SYSTEM_CAN_WRITE] = devfs_can_write;
    fs_desc.functions[DRIVER_FILE_SYSTEM_OPEN] = devfs_open;
    fs_desc.functions[DRIVER_FILE_SYSTEM_READ] = devfs_read;
    fs_desc.functions[DRIVER_FILE_SYSTEM_WRITE] = devfs_write;
    fs_desc.functions[DRIVER_FILE_SYSTEM_TRUNCATE] = NULL;
    fs_desc.functions[DRIVER_FILE_SYSTEM_MKDIR] = devfs_mkdir_dummy;
    fs_desc.functions[DRIVER_FILE_SYSTEM_RMDIR] = devfs_rmdir_dummy;
    fs_desc.functions[DRIVER_FILE_SYSTEM_EJECT_DEVICE] = NULL;

    fs_desc.functions[DRIVER_FILE_SYSTEM_READ_INODE] = devfs_read_inode;
    fs_desc.functions[DRIVER_FILE_SYSTEM_WRITE_INODE] = devfs_write_inode;
    fs_desc.functions[DRIVER_FILE_SYSTEM_FREE_INODE] = devfs_free_inode;
    fs_desc.functions[DRIVER_FILE_SYSTEM_LOOKUP] = devfs_lookup;
    fs_desc.functions[DRIVER_FILE_SYSTEM_GETDENTS] = devfs_getdents;
    fs_desc.functions[DRIVER_FILE_SYSTEM_CREATE] = NULL;
    fs_desc.functions[DRIVER_FILE_SYSTEM_UNLINK] = NULL;
    fs_desc.functions[DRIVER_FILE_SYSTEM_FSTAT] = devfs_fstat;
    fs_desc.functions[DRIVER_FILE_SYSTEM_FCHMOD] = devfs_fchmod;
    fs_desc.functions[DRIVER_FILE_SYSTEM_IOCTL] = devfs_ioctl;
    fs_desc.functions[DRIVER_FILE_SYSTEM_MMAP] = devfs_mmap;

    return fs_desc;
}

void devfs_install()
{
    devman_register_driver(_devfs_driver_info(), "devfs");
}
devman_register_driver_installation(devfs_install);

/**
 * Register a device.
 */

devfs_inode_t* devfs_mkdir(const path_t* vfspath, const char* name, size_t len)
{
    spinlock_acquire(&_devfs_lock);
    devfs_inode_t* devfs_inode = (devfs_inode_t*)vfspath->dentry->inode;
    devfs_inode_t* new_entry = _devfs_alloc_entry(devfs_inode);
    if (!new_entry) {
        spinlock_release(&_devfs_lock);
        return 0;
    }

    dentry_set_flag(vfspath->dentry, DENTRY_DIRTY);

    new_entry->mode = S_IFDIR;
    _devfs_set_name(new_entry, name, len);

    spinlock_release(&_devfs_lock);
    return new_entry;
}

devfs_inode_t* devfs_register(const path_t* vfspath, dev_t devid, const char* name, size_t len, mode_t mode, const file_ops_t* handlers)
{
    spinlock_acquire(&_devfs_lock);
    devfs_inode_t* devfs_inode = (devfs_inode_t*)vfspath->dentry->inode;
    devfs_inode_t* new_entry = _devfs_alloc_entry(devfs_inode);
    if (!new_entry) {
        spinlock_release(&_devfs_lock);
        return NULL;
    }

    new_entry->dev_id = devid;
    new_entry->mode = mode;
    _devfs_set_name(new_entry, name, len);
    _devfs_set_handlers(new_entry, handlers);
    dentry_set_flag(vfspath->dentry, DENTRY_DIRTY);

    spinlock_release(&_devfs_lock);
    return new_entry;
}

int devfs_mount()
{
    spinlock_init(&_devfs_lock);
    path_t vfspth;
    if (vfs_resolve_path("/dev", &vfspth) < 0) {
        return -ENOENT;
    }
    int driver_id = vfs_get_fs_id("devfs");
    if (driver_id < 0) {
        log("Devfs: no driver is installed, exiting");
        return -ENOENT;
    }

    int err = vfs_mount(&vfspth, new_virtual_device(DEVICE_STORAGE), driver_id);
    path_put(&vfspth);
    if (!err) {
        devman_send_notification(DEVMAN_NOTIFICATION_DEVFS_READY, 0);
    }
    return err;
}