/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <algo/dynamic_array.h>
#include <errno.h>
#include <fs/devfs/devfs.h>
#include <mem/kmalloc.h>

#define DEVFS_ZONE_SIZE 32 * KB

static dynamic_array_t entry_zones; /* Just store pointers to free zones one time. */
static void* next_space_to_put_entry;
static uint32_t free_space_in_last_entry_zone;

static dynamic_array_t name_zones; /* Just store pointers to free zones one time. */
static void* next_space_to_put_name;
static uint32_t free_space_in_last_name_zone;

static devfs_inode_t* devfs_root;

static uint32_t next_inode_index = 2;

/**
 * Zones Managment
 */

static int _devfs_alloc_entry_zone()
{
    void* new_zone = kmalloc(DEVFS_ZONE_SIZE);
    if (!new_zone) {
        return ENOMEM;
    }

    dynamic_array_push(&entry_zones, &new_zone);
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

    dynamic_array_push(&name_zones, &new_zone);
    next_space_to_put_name = new_zone;
    free_space_in_last_name_zone = DEVFS_ZONE_SIZE;
    return 0;
}

static devfs_inode_t* _devfs_get_entry(uint32_t indx)
{
    uint32_t entries_per_zone = DEVFS_ZONE_SIZE / DEVFS_INODE_LEN;
    uint32_t zones_count = entry_zones.size;
    if (indx >= zones_count * entries_per_zone) {
        return 0;
    }

    uint32_t requested_zone = indx / entries_per_zone;
    uint32_t index_within_zone = indx % entries_per_zone;
    uint32_t* tmp_ptr = dynamic_array_get(&entry_zones, requested_zone);
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
    res->index = next_inode_index++;
    next_space_to_put_entry += sizeof(devfs_inode_t);
    free_space_in_last_entry_zone -= sizeof(devfs_inode_t);
    return res;
}

static char* _devfs_new_name(int len)
{
    if (free_space_in_last_entry_zone < len + 1) {
        _devfs_alloc_name_zone();
    }

    char* res = (char*)next_space_to_put_entry;
    next_space_to_put_entry += len + 1;
    free_space_in_last_entry_zone -= len + 1;
    memset(res, 0, len + 1);
    return res;
}

static inline devfs_inode_t* _devfs_get_devfs_inode(uint32_t inode_indx)
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

static int _devfs_set_name(devfs_inode_t* entry, const char* name, uint32_t len)
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

/**
 * FS Tools
 */

static int _devfs_setup_root()
{
    devfs_root = _devfs_new_entry();
    memset((void*)devfs_root, 0, DEVFS_INODE_LEN);
    devfs_root->index = 2;
    devfs_root->mode = EXT2_S_IFDIR;
    return 0;
}

/**
 * VFS Api
 */

fsdata_t devfs_data(dentry_t* dentry)
{
    /* Set everything to 0, since devfs isn't supposed to be used by several devices.
       All of mount of devfs will show the same info. */
    fsdata_t fsdata;
    fsdata.sb = 0;
    fsdata.gt = 0;
    return fsdata;
}

int devfs_prepare_fs(vfs_device_t* vdev)
{
    dynamic_array_init(&name_zones, sizeof(void*));
    dynamic_array_init(&entry_zones, sizeof(void*));
    _devfs_setup_root();
    return 0;
}

int devfs_read_inode(dentry_t* dentry)
{
    /*  We currently has a uniqueue structure of inode for devfs. So we need to be
        confident using inode in vfs, since inode and devfs_inode are not similar. */
    devfs_inode_t* devfs_inode = _devfs_get_devfs_inode(dentry->inode_indx);
    if (!devfs_inode) {
        return -EFAULT;
    }
    memcpy((void*)dentry->inode, (void*)devfs_inode, DEVFS_INODE_LEN);
    return 0;
}

int devfs_write_inode(dentry_t* dentry)
{
    devfs_inode_t* devfs_inode = _devfs_get_devfs_inode(dentry->inode_indx);
    if (!devfs_inode) {
        return -EFAULT;
    }
    memcpy((void*)devfs_inode, (void*)dentry->inode, DEVFS_INODE_LEN);
    return 0;
}

int devfs_getdents(dentry_t* dir, uint8_t* buf, uint32_t* offset, uint32_t len)
{
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
        int rec_len = 8 + 1 + 1;

        if (len < rec_len) {
            if (already_read) {
                return already_read;
            } else {
                return -EINVAL;
            }
        }

        tmp.inode = devfs_inode->index;
        tmp.rec_len = rec_len;
        tmp.name_len = 1;
        memcpy(buf + already_read, (void*)&tmp, 8);
        memcpy(buf + already_read + 8, ".", 1);
        buf[already_read + rec_len - 1] = '\0';
        already_read += rec_len;
        len -= rec_len;
        *offset = 1;
    }

    /* Return .. */
    if (*offset == 1) {
        int rec_len = 8 + 2 + 1;

        if (len < rec_len) {
            if (already_read) {
                return already_read;
            } else {
                return -EINVAL;
            }
        }

        if (devfs_inode->parent) {
            tmp.inode = devfs_inode->parent->index;
        } else {
            tmp.inode = devfs_inode->index;
        }
        tmp.rec_len = rec_len;
        tmp.name_len = 2;
        memcpy(buf + already_read, (void*)&tmp, 8);
        memcpy(buf + already_read + 8, "..", 2);
        buf[already_read + rec_len - 1] = '\0';
        already_read += rec_len;
        len -= rec_len;
        *offset = 2;
    }

    /* Scanining dir from the start */
    if (*offset == 2) {
        if (!devfs_inode->first) {
            return 0;
        }
        *offset = (uint32_t)devfs_inode->first;
    }

    while (*offset != 0xffffffff) {
        devfs_inode_t* child_devfs_inode = (devfs_inode_t*)*offset;
        uint32_t name_len = strlen(child_devfs_inode->name);
        int rec_len = 8 + name_len + 1;

        if (len < rec_len) {
            if (already_read) {
                return already_read;
            } else {
                return -EINVAL;
            }
        }

        tmp.inode = child_devfs_inode->index;
        tmp.rec_len = rec_len;
        tmp.name_len = name_len;
        memcpy(buf + already_read, (void*)&tmp, 8);
        memcpy(buf + already_read + 8, child_devfs_inode->name, name_len);
        buf[already_read + rec_len - 1] = '\0';
        already_read += rec_len;
        len -= rec_len;
        if (child_devfs_inode->next) {
            *offset = (uint32_t)child_devfs_inode->next;
        } else {
            *offset = 0xffffffff;
        }
    }

    return already_read;
}

int devfs_lookup(dentry_t* dir, const char* name, uint32_t len, uint32_t* res_inode_indx)
{
    devfs_inode_t* devfs_inode = (devfs_inode_t*)dir->inode;

    if (len == 1) {
        if (name[0] == '.') {
            *res_inode_indx = devfs_inode->index;
            return 0;
        }
    }

    if (len == 2) {
        if (name[0] == '.' && name[1] == '.') {
            *res_inode_indx = devfs_inode->parent->index;
            return 0;
        }
    }

    devfs_inode_t* child = devfs_inode->first;
    while (child) {
        uint32_t child_name_len = strlen(child->name);
        if (len == child_name_len) {
            if (strncmp(name, child->name, len) == 0) {
                *res_inode_indx = child->index;
                return 0;
            }
        }
        child = child->next;
    }

    return -ENOENT;
}

int devfs_mkdir_dummy(dentry_t* dir, const char* name, uint32_t len, mode_t mode)
{
    return -1;
}

int devfs_can_read(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len)
{
    devfs_inode_t* devfs_inode = (devfs_inode_t*)dentry->inode;
    if (devfs_inode->handlers.can_read) {
        return devfs_inode->handlers.can_read(dentry);
    }
    return true;
}

int devfs_can_write(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len)
{
    devfs_inode_t* devfs_inode = (devfs_inode_t*)dentry->inode;
    if (devfs_inode->handlers.can_write) {
        return devfs_inode->handlers.can_write(dentry);
    }
    return true;
}

int devfs_read(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len)
{
    devfs_inode_t* devfs_inode = (devfs_inode_t*)dentry->inode;
    if (devfs_inode->handlers.read) {
        return devfs_inode->handlers.read(dentry, buf, start, len);
    }
    return -EFAULT;
}

int devfs_write(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len)
{
    devfs_inode_t* devfs_inode = (devfs_inode_t*)dentry->inode;
    if (devfs_inode->handlers.write) {
        return devfs_inode->handlers.write(dentry, buf, start, len);
    }
    return -EFAULT;
}

int devfs_ioctl(dentry_t* dentry, uint32_t cmd, uint32_t arg)
{
    devfs_inode_t* devfs_inode = (devfs_inode_t*)dentry->inode;
    if (devfs_inode->handlers.ioctl) {
        return devfs_inode->handlers.ioctl(dentry, cmd, arg);
    }
    return -EFAULT;
}

/**
 * Driver install functions.
 */

driver_desc_t _devfs_driver_info()
{
    driver_desc_t fs_desc;
    fs_desc.type = DRIVER_FILE_SYSTEM;
    fs_desc.auto_start = false;
    fs_desc.is_device_driver = false;
    fs_desc.is_device_needed = false;
    fs_desc.is_driver_needed = false;
    fs_desc.functions[DRIVER_FILE_SYSTEM_RECOGNIZE] = 0;
    fs_desc.functions[DRIVER_FILE_SYSTEM_PREPARE_FS] = devfs_prepare_fs;
    fs_desc.functions[DRIVER_FILE_SYSTEM_CAN_READ] = devfs_can_read;
    fs_desc.functions[DRIVER_FILE_SYSTEM_CAN_WRITE] = devfs_can_write;
    fs_desc.functions[DRIVER_FILE_SYSTEM_READ] = devfs_read;
    fs_desc.functions[DRIVER_FILE_SYSTEM_WRITE] = devfs_write;
    fs_desc.functions[DRIVER_FILE_SYSTEM_MKDIR] = devfs_mkdir_dummy;
    fs_desc.functions[DRIVER_FILE_SYSTEM_EJECT_DEVICE] = 0;

    fs_desc.functions[DRIVER_FILE_SYSTEM_READ_INODE] = devfs_read_inode;
    fs_desc.functions[DRIVER_FILE_SYSTEM_WRITE_INODE] = devfs_write_inode;
    fs_desc.functions[DRIVER_FILE_SYSTEM_GET_FSDATA] = devfs_data;
    fs_desc.functions[DRIVER_FILE_SYSTEM_LOOKUP] = devfs_lookup;
    fs_desc.functions[DRIVER_FILE_SYSTEM_GETDENTS] = devfs_getdents;
    fs_desc.functions[DRIVER_FILE_SYSTEM_CREATE] = 0;
    fs_desc.functions[DRIVER_FILE_SYSTEM_RM] = 0;
    fs_desc.functions[DRIVER_FILE_SYSTEM_IOCTL] = devfs_ioctl;

    return fs_desc;
}

void devfs_install()
{
    driver_install(_devfs_driver_info());
}

/**
 * Register a device.
 */

devfs_inode_t* devfs_mkdir(dentry_t* dir, const char* name, uint32_t len)
{
    devfs_inode_t* devfs_inode = (devfs_inode_t*)dir->inode;
    devfs_inode_t* new_entry = _devfs_alloc_entry(devfs_inode);
    if (!new_entry) {
        return 0;
    }

    dentry_set_flag(dir, DENTRY_DIRTY);

    new_entry->mode = EXT2_S_IFDIR;
    _devfs_set_name(new_entry, name, len);

    return new_entry;
}

devfs_inode_t* devfs_register(dentry_t* dir, const char* name, uint32_t len, mode_t mode, const file_ops_t* handlers)
{
    devfs_inode_t* devfs_inode = (devfs_inode_t*)dir->inode;
    devfs_inode_t* new_entry = _devfs_alloc_entry(devfs_inode);
    if (!new_entry) {
        return 0;
    }

    dentry_set_flag(dir, DENTRY_DIRTY);

    new_entry->mode = mode;
    _devfs_set_name(new_entry, name, len);
    memcpy((void*)&new_entry->handlers, (void*)handlers, sizeof(*handlers));

    return new_entry;
}

int devfs_mount()
{
    dentry_t* mp;
    if (vfs_resolve_path("/dev", &mp) < 0) {
        return -ENOENT;
    }
    int err = vfs_mount(mp, new_virtual_device(DEVICE_STORAGE), 2);
    dentry_put(mp);
    return err;
}