/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <drivers/display.h>
#include <fs/vfs.h>
#include <mem/kmalloc.h>
#include <utils/mem.h>

vfs_device_t _vfs_devices[MAX_DEVICES_COUNT];
fs_ops_t _vfs_fses[MAX_DRIVERS_COUNT];
uint8_t _vfs_fses_count; // Will be deleted in next builds
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
    vfs_desc.functions[DRIVER_VIRTUAL_FILE_SYSTEM_ADD_DEVICE] = vfs_add_device;
    vfs_desc.functions[DRIVER_VIRTUAL_FILE_SYSTEM_ADD_DRIVER] = vfs_add_fs;
    vfs_desc.functions[DRIVER_VIRTUAL_FILE_SYSTEM_EJECT_DEVICE] = vfs_eject_device;
    return vfs_desc;
}

void vfs_install()
{
    driver_install(_vfs_driver_info());
}

int vfs_detect_fs_of_dev(vfs_device_t* vfs_dev)
{
    for (uint8_t i = 0; i < _vfs_fses_count; i++) {
        if (!_vfs_fses[i].recognize) {
            continue;
        }

        bool (*is_capable)(vfs_device_t * nd) = _vfs_fses[i].recognize;
        if (is_capable(vfs_dev)) {
            vfs_dev->fs = i;
            return 0;
        }
    }
    return -1;
}

void vfs_add_device(device_t* dev)
{
    if (dev->type != DEVICE_STORAGE) {
        return;
    }
    
    if (root_fs_dev_id == -1) {
        root_fs_dev_id = dev->id;
    }
    _vfs_devices[dev->id].dev = dev;
    if (!dev->is_virtual) {
        vfs_detect_fs_of_dev(&_vfs_devices[dev->id]);
    }
}

// TODO: reuse unused slots
void vfs_eject_device(device_t* dev)
{
    kprintf("Ejecting\n");
    uint8_t fs_id = _vfs_devices[dev->id].fs;
    bool (*eject)(vfs_device_t * nd) = _vfs_fses[fs_id].eject_device;
    eject(&_vfs_devices[dev->id]);
    dentry_put_all_dentries_of_dev(dev->id);
}

void vfs_add_fs(driver_t* t_new_driver)
{
    if (t_new_driver->driver_desc.type != DRIVER_FILE_SYSTEM) {
        return;
    }

    fs_ops_t new_fs;

    new_fs.recognize = t_new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_RECOGNIZE];
    new_fs.eject_device = t_new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_EJECT_DEVICE];

    new_fs.file.mkdir = t_new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_MKDIR];
    new_fs.file.getdirent = t_new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_GETDIRENT]; // TODO FIX
    new_fs.file.lookup = t_new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_LOOKUP];
    new_fs.file.read = t_new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_READ];
    new_fs.file.write = t_new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_WRITE];

    new_fs.dentry.write_inode = t_new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_WRITE_INODE];
    new_fs.dentry.read_inode = t_new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_READ_INODE];
    new_fs.dentry.get_fsdata = t_new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_GET_FSDATA];

    _vfs_fses[_vfs_fses_count++] = new_fs;
}

int vfs_open(dentry_t* file, file_descriptor_t* fd)
{
    if (!file) {
        return -1;
    }

    if (!fd) {
        return -1;
    }

    fd->dentry = dentry_duplicate(file);
    fd->offset = 0;
    fd->ops = &file->ops->file;
    return 0;
}

int vfs_close(file_descriptor_t* fd)
{
    if (!fd) {
        return -1;
    }

    dentry_put(fd->dentry);
    fd->dentry = 0;
    fd->offset = 0;
    fd->ops = 0;
    return 0;
}

int vfs_lookup(dentry_t* dir, const char* name, uint32_t len, dentry_t** result)
{
    if (!dentry_inode_test_flag(dir, EXT2_S_IFDIR)) {
        return -1;
    }

    uint32_t next_inode;
    if (dir->ops->file.lookup(dir, name, len, &next_inode) == 0) {
        *result = dentry_get(root_fs_dev_id, next_inode);
        return 0;
    }
    return -1;
}

int vfs_read(file_descriptor_t* fd, uint8_t* buf, uint32_t start, uint32_t len)
{
    return fd->ops->read(fd->dentry, buf, start, len);
}

int vfs_write(file_descriptor_t* fd, uint8_t* buf, uint32_t start, uint32_t len)
{
    return fd->ops->write(fd->dentry, buf, start, len);
}

int vfs_mkdir(dentry_t* dir, const char* name, uint32_t len, uint16_t mode)
{
    if (!dentry_inode_test_flag(dir, EXT2_S_IFDIR)) {
        return -1;
    }
    return dir->ops->file.mkdir(dir, name, len, mode);
}

int vfs_getdirent(file_descriptor_t* dir_fd, dirent_t *res)
{
    if (!dentry_inode_test_flag(dir_fd->dentry, EXT2_S_IFDIR)) {
        return -1;
    }
    return dir_fd->ops->getdirent(dir_fd->dentry, &dir_fd->offset, res);
}

int vfs_resolve_path_start_from(dentry_t* dentry, const char* path, dentry_t** result)
{
    if (!path) {
        return -1;
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
            return -1;
        }
        
        while (dentry_test_flag(cur_dent, DENTRY_MOUNTPOINT)) {
            cur_dent = cur_dent->mounted_dentry;
        }

        dentry_set_parent(cur_dent, parent_dent);
        // dentry_put(parent_dent);
    }

    *result = dentry_duplicate(cur_dent);
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
        return -1;
    }
    if (!dentry_inode_test_flag(mountpoint, EXT2_S_IFDIR)) {
        kprintf("Not a dir\n");
        return -1;
    }

    vfs_add_device(dev);
    _vfs_devices[dev->id].fs = fs_indx;

    mountpoint = dentry_duplicate(mountpoint); // to keep mounts in mem until to umount
    dentry_set_flag(mountpoint, DENTRY_MOUNTPOINT);
    
    dentry_t* mounted_dentry = dentry_get(dev->id, 2);
    mounted_dentry = dentry_duplicate(mounted_dentry); // to keep mounts in mem until to umount
    dentry_set_flag(mounted_dentry, DENTRY_MOUNTED);
    
    mountpoint->mounted_dentry = mounted_dentry;
    mounted_dentry->mountpoint = mountpoint;

    return 0;
}

int vfs_umount(dentry_t* mounted_dentry)
{
    if (!dentry_test_flag(mounted_dentry, DENTRY_MOUNTED)) {
        kprintf("Not mounted\n");
        return -1;
    }

    dentry_t* mountpoint = mounted_dentry->mountpoint;

    if (!dentry_test_flag(mountpoint, DENTRY_MOUNTPOINT)) {
        kprintf("Not a mountpoint\n");
        return -1;
    }

    dentry_rem_flag(mounted_dentry, DENTRY_MOUNTED);
    mounted_dentry->mountpoint = 0;
    dentry_put(mounted_dentry);
    
    dentry_rem_flag(mountpoint, DENTRY_MOUNTPOINT);
    mountpoint->mounted_dentry = 0;
    dentry_put(mountpoint);

    if (dentry_test_flag(mountpoint, DENTRY_MOUNTED)) {
        vfs_umount(mountpoint);
    }

    return 0;
}