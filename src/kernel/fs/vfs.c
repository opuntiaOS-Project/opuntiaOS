/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <algo/dynamic_array.h>
#include <drivers/display.h>
#include <errno.h>
#include <fs/vfs.h>
#include <io/sockets/socket.h>
#include <mem/kmalloc.h>
#include <utils/mem.h>

vfs_device_t _vfs_devices[MAX_DEVICES_COUNT];
dynamic_array_t _vfs_fses;
int32_t root_fs_dev_id = -1;

/**
 * DENTRY CACHES
 */

driver_desc_t _vfs_driver_info()
{
    driver_desc_t vfs_desc;
    vfs_desc.type = DRIVER_VIRTUAL_FILE_SYSTEM;
    vfs_desc.is_device_driver = false;
    vfs_desc.is_device_needed = true;
    vfs_desc.is_driver_needed = true;
    vfs_desc.type_of_needed_device = DEVICE_STORAGE;
    vfs_desc.type_of_needed_driver = DRIVER_FILE_SYSTEM;
    vfs_desc.functions[DRIVER_VIRTUAL_FILE_SYSTEM_ADD_DEVICE] = vfs_add_dev;
    vfs_desc.functions[DRIVER_VIRTUAL_FILE_SYSTEM_ADD_DRIVER] = vfs_add_fs;
    vfs_desc.functions[DRIVER_VIRTUAL_FILE_SYSTEM_EJECT_DEVICE] = vfs_eject_device;
    return vfs_desc;
}

/**
 * Init of vfs
 */
void vfs_install()
{
    driver_install(_vfs_driver_info());
    dynamic_array_init_of_size(&_vfs_fses, sizeof(fs_ops_t), 16);
}

int vfs_choose_fs_of_dev(vfs_device_t* vfs_dev)
{
    int fs_cnt = _vfs_fses.size;
    for (int i = 0; i < fs_cnt; i++) {
        fs_ops_t* fs = dynamic_array_get(&_vfs_fses, (int)i);
        if (!fs->recognize) {
            continue;
        }

        bool (*is_capable)(vfs_device_t * nd) = fs->recognize;
        if (is_capable(vfs_dev)) {
            vfs_dev->fs = i;
            if (fs->prepare_fs) {
                int (*prepare_fs)(vfs_device_t * nd) = fs->prepare_fs;
                return prepare_fs(&_vfs_devices[vfs_dev->dev->id]);
            }
            return 0;
        }
    }
    return -ENOENT;
}

int vfs_add_dev(device_t* dev)
{
    if (dev->type != DEVICE_STORAGE) {
        return -EPERM;
    }

    if (root_fs_dev_id == -1) {
        root_fs_dev_id = dev->id;
    }

    _vfs_devices[dev->id].dev = dev;
    if (!dev->is_virtual) {
        if (vfs_choose_fs_of_dev(&_vfs_devices[dev->id]) < 0) {
            return -ENOENT;
        }
    }
    return 0;
}

int vfs_add_dev_with_fs(device_t* dev, int fs_id)
{
    if (dev->type != DEVICE_STORAGE) {
        return -EPERM;
    }

    if (root_fs_dev_id == -1) {
        root_fs_dev_id = dev->id;
    }

    _vfs_devices[dev->id].dev = dev;
    _vfs_devices[dev->id].fs = fs_id;

    fs_ops_t* fs = dynamic_array_get(&_vfs_fses, fs_id);
    if (fs->prepare_fs) {
        int (*prepare_fs)(vfs_device_t * nd) = fs->prepare_fs;
        return prepare_fs(&_vfs_devices[dev->id]);
    }

    return 0;
}

// TODO: reuse unused slots
void vfs_eject_device(device_t* dev)
{
    kprintf("Ejecting\n");
    int fs_id = _vfs_devices[dev->id].fs;
    fs_ops_t* fs = dynamic_array_get(&_vfs_fses, (int)fs_id);
    if (fs->eject_device) {
        bool (*eject)(vfs_device_t * nd) = fs->eject_device;
        eject(&_vfs_devices[dev->id]);
    }
    dentry_put_all_dentries_of_dev(dev->id);
}

void vfs_add_fs(driver_t* new_driver)
{
    if (new_driver->driver_desc.type != DRIVER_FILE_SYSTEM) {
        return;
    }

    fs_ops_t new_fs;

    new_fs.recognize = new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_RECOGNIZE];
    new_fs.prepare_fs = new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_PREPARE_FS];
    new_fs.eject_device = new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_EJECT_DEVICE];

    new_fs.file.mkdir = new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_MKDIR];
    new_fs.file.getdirent = new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_GETDIRENT];
    new_fs.file.lookup = new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_LOOKUP];
    new_fs.file.can_read = new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_CAN_READ];
    new_fs.file.can_write = new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_CAN_WRITE];
    new_fs.file.read = new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_READ];
    new_fs.file.write = new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_WRITE];
    new_fs.file.create = new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_CREATE];
    new_fs.file.rm = new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_RM];

    new_fs.dentry.write_inode = new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_WRITE_INODE];
    new_fs.dentry.read_inode = new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_READ_INODE];
    new_fs.dentry.get_fsdata = new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_GET_FSDATA];

    dynamic_array_push(&_vfs_fses, &new_fs);
}

int vfs_open(dentry_t* file, file_descriptor_t* fd)
{
    if (!file) {
        return -EFAULT;
    }

    if (!fd) {
        return -EFAULT;
    }

    fd->type = FD_TYPE_FILE;
    fd->dentry = dentry_duplicate(file);
    fd->offset = 0;
    fd->ops = &file->ops->file;
    return 0;
}

int vfs_close(file_descriptor_t* fd)
{
    if (!fd) {
        return -EFAULT;
    }

    if (fd->type == FD_TYPE_FILE) {
        dentry_put(fd->dentry);
    } else {
        socket_free(fd->sock_entry);
    }
    fd->dentry = 0;
    fd->offset = 0;
    fd->ops = 0;
    return 0;
}

int vfs_create(dentry_t* dir, const char* name, uint32_t len, mode_t mode)
{
    /* Check if there is a file with the same name */
    dentry_t* tmp;
    if (vfs_lookup(dir, name, len, &tmp) == 0) {
        return -EEXIST;
    }

    return dir->ops->file.create(dir, name, len, mode);
}

int vfs_rm(dentry_t* file)
{
    if (file->d_count != 1) {
        kprintf("d_count isn't 1, but %d\n", file->d_count);
        return -EPERM;
    }

    return file->ops->file.rm(file);
}

int vfs_lookup(dentry_t* dir, const char* name, uint32_t len, dentry_t** result)
{
    if (!dentry_inode_test_flag(dir, EXT2_S_IFDIR)) {
        return -ENOTDIR;
    }

    uint32_t next_inode;
    if (dir->ops->file.lookup(dir, name, len, &next_inode) == 0) {
        *result = dentry_get(dir->dev_indx, next_inode);
        return 0;
    }
    return -ENOENT;
}

bool vfs_can_read(file_descriptor_t* fd, uint8_t* buf, uint32_t start, uint32_t len)
{
    return fd->ops->can_read(fd->dentry);
}

bool vfs_can_write(file_descriptor_t* fd, uint8_t* buf, uint32_t start, uint32_t len)
{
    return fd->ops->can_write(fd->dentry);
}

int vfs_read(file_descriptor_t* fd, uint8_t* buf, uint32_t start, uint32_t len)
{
    return fd->ops->read(fd->dentry, buf, start, len);
}

int vfs_write(file_descriptor_t* fd, uint8_t* buf, uint32_t start, uint32_t len)
{
    return fd->ops->write(fd->dentry, buf, start, len);
}

int vfs_mkdir(dentry_t* dir, const char* name, uint32_t len, mode_t mode)
{
    if (!dentry_inode_test_flag(dir, EXT2_S_IFDIR)) {
        return -ENOTDIR;
    }
    return dir->ops->file.mkdir(dir, name, len, mode | EXT2_S_IFDIR);
}

int vfs_getdirent(file_descriptor_t* dir_fd, dirent_t* res)
{
    if (!dentry_inode_test_flag(dir_fd->dentry, EXT2_S_IFDIR)) {
        return -ENOTDIR;
    }
    return dir_fd->ops->getdirent(dir_fd->dentry, &dir_fd->offset, res);
}

int vfs_resolve_path_start_from(dentry_t* dentry, const char* path, dentry_t** result)
{
    if (!path) {
        return -EFAULT;
    }

    dentry_t* cur_dent;

    if (!dentry || path[0] == '/') {
        cur_dent = dentry_get(root_fs_dev_id, 2);
        while (*path == '/')
            path++;
    }

    while (*path != '\0') {
        while (*path == '/')
            path++;
        const char* name = path;

        int len = 0;
        while (*path != '\0' && *path != '/')
            path++, len++;

        dentry_t* parent_dent = cur_dent;
        if (vfs_lookup(cur_dent, name, len, &cur_dent) < 0) {
            return -ENOENT;
        }

        dentry_t* lookuped_dent = cur_dent;
        while (dentry_test_flag(cur_dent, DENTRY_MOUNTPOINT)) {
            cur_dent = cur_dent->mounted_dentry;
        }

        if (dentry_test_flag(lookuped_dent, DENTRY_MOUNTPOINT)) {
            dentry_put(lookuped_dent);
            cur_dent = dentry_duplicate(cur_dent);
        }

        dentry_set_parent(cur_dent, parent_dent);
        dentry_put(parent_dent);
    }

    *result = dentry_duplicate(cur_dent);
    dentry_put(cur_dent);
    return 0;
}

int vfs_resolve_path(const char* path, dentry_t** result)
{
    return vfs_resolve_path_start_from((dentry_t*)0, path, result);
}

int vfs_mount(dentry_t* mountpoint, device_t* dev, uint32_t fs_indx)
{
    if (dentry_test_flag(mountpoint, DENTRY_MOUNTPOINT)) {
        kprintf("Already a mount point\n");
        return -EBUSY;
    }
    if (!dentry_inode_test_flag(mountpoint, EXT2_S_IFDIR)) {
        kprintf("Not a dir\n");
        return -ENOTDIR;
    }

    vfs_add_dev_with_fs(dev, fs_indx);

    mountpoint = dentry_duplicate(mountpoint); /* We keep mounts in mem until to umount. */
    dentry_set_flag(mountpoint, DENTRY_MOUNTPOINT);

    dentry_t* mounted_dentry = dentry_get(dev->id, 2); /* Not going to put it, to keep mounts in mem until to umount */
    dentry_set_flag(mounted_dentry, DENTRY_MOUNTED);

    mountpoint->mounted_dentry = mounted_dentry;
    mounted_dentry->mountpoint = mountpoint;

    return 0;
}

int vfs_umount(dentry_t* mounted_dentry)
{
    if (!dentry_test_flag(mounted_dentry, DENTRY_MOUNTED)) {
        kprintf("Not mounted\n");
        return -EPERM;
    }

    dentry_t* mountpoint = mounted_dentry->mountpoint;

    if (!dentry_test_flag(mountpoint, DENTRY_MOUNTPOINT)) {
        kprintf("Not a mountpoint\n");
        return -EPERM;
    }

    dentry_rem_flag(mounted_dentry, DENTRY_MOUNTED);
    dentry_rem_flag(mountpoint, DENTRY_MOUNTPOINT);

    mounted_dentry->mountpoint = 0;
    mountpoint->mounted_dentry = 0;

    dentry_put(mounted_dentry);
    dentry_put(mountpoint);

    if (dentry_test_flag(mountpoint, DENTRY_MOUNTED)) {
        vfs_umount(mountpoint);
    }

    return 0;
}