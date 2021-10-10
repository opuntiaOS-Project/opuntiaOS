/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <algo/dynamic_array.h>
#include <fs/vfs.h>
#include <io/sockets/socket.h>
#include <libkern/bits/errno.h>
#include <libkern/libkern.h>
#include <libkern/log.h>
#include <libkern/syscall_structs.h>
#include <mem/kmalloc.h>
#include <tasking/cpu.h>
#include <tasking/proc.h>
#include <tasking/tasking.h>

// #define VFS_DEBUG
#define MAX_FS 8

vfs_device_t _vfs_devices[MAX_DEVICES_COUNT];
dynamic_array_t _vfs_fses;
int32_t root_fs_dev_id = -1;

driver_desc_t _vfs_driver_info()
{
    driver_desc_t vfs_desc = { 0 };
    vfs_desc.type = DRIVER_VIRTUAL_FILE_SYSTEM;
    vfs_desc.is_device_driver = false;
    vfs_desc.is_device_needed = true;
    vfs_desc.is_driver_needed = true;
    vfs_desc.type_of_needed_device = DEVICE_STORAGE;
    vfs_desc.type_of_needed_driver = DRIVER_FILE_SYSTEM;
    vfs_desc.functions[DRIVER_NOTIFICATION] = 0;
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
    driver_install(_vfs_driver_info(), "vfs");
    dynarr_init_of_size(fs_desc_t, &_vfs_fses, MAX_FS);
}

int vfs_choose_fs_of_dev(vfs_device_t* vfs_dev)
{
    int fs_cnt = _vfs_fses.size;
    for (int i = 0; i < fs_cnt; i++) {
        fs_desc_t* fs = dynarr_get(&_vfs_fses, (int)i);
        if (!fs->ops->recognize) {
            continue;
        }

        int err = fs->ops->recognize(vfs_dev);
        if (!err) {
            vfs_dev->fs = i;
            if (fs->ops->prepare_fs) {
                return fs->ops->prepare_fs(&_vfs_devices[vfs_dev->dev->id]);
            }
            return 0;
        }
    }
    return -ENOENT;
}

int vfs_get_fs_id(const char* name)
{
    int fs_cnt = _vfs_fses.size;
    for (int i = 0; i < fs_cnt; i++) {
        fs_desc_t* fs = dynarr_get(&_vfs_fses, (int)i);
        if (strcmp(name, fs->driver->name) == 0) {
            return i;
        }
    }
    return -1;
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
    lock_init(&_vfs_devices[dev->id].lock);
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

    fs_desc_t* fs = dynarr_get(&_vfs_fses, fs_id);
    if (fs->ops->prepare_fs) {
        int (*prepare_fs)(vfs_device_t * nd) = fs->ops->prepare_fs;
        return prepare_fs(&_vfs_devices[dev->id]);
    }

    return 0;
}

// TODO: reuse unused slots
void vfs_eject_device(device_t* dev)
{
#ifdef VFS_DEBUG
    log("Ejecting\n");
#endif
    int fs_id = _vfs_devices[dev->id].fs;
    fs_desc_t* fs = dynarr_get(&_vfs_fses, (int)fs_id);
    if (fs->ops->eject_device) {
        int (*eject)(vfs_device_t * nd) = fs->ops->eject_device;
        eject(&_vfs_devices[dev->id]);
    }
    dentry_put_all_dentries_of_dev(dev->id);
}

void vfs_add_fs(driver_t* new_driver)
{
    if (new_driver->desc.type != DRIVER_FILE_SYSTEM) {
        return;
    }

    fs_ops_t* new_ops = kmalloc(sizeof(fs_ops_t));

    new_ops->recognize = new_driver->desc.functions[DRIVER_FILE_SYSTEM_RECOGNIZE];
    new_ops->prepare_fs = new_driver->desc.functions[DRIVER_FILE_SYSTEM_PREPARE_FS];
    new_ops->eject_device = new_driver->desc.functions[DRIVER_FILE_SYSTEM_EJECT_DEVICE];

    new_ops->file.mkdir = new_driver->desc.functions[DRIVER_FILE_SYSTEM_MKDIR];
    new_ops->file.rmdir = new_driver->desc.functions[DRIVER_FILE_SYSTEM_RMDIR];
    new_ops->file.getdents = new_driver->desc.functions[DRIVER_FILE_SYSTEM_GETDENTS];
    new_ops->file.lookup = new_driver->desc.functions[DRIVER_FILE_SYSTEM_LOOKUP];
    new_ops->file.open = new_driver->desc.functions[DRIVER_FILE_SYSTEM_OPEN];
    new_ops->file.can_read = new_driver->desc.functions[DRIVER_FILE_SYSTEM_CAN_READ];
    new_ops->file.can_write = new_driver->desc.functions[DRIVER_FILE_SYSTEM_CAN_WRITE];
    new_ops->file.read = new_driver->desc.functions[DRIVER_FILE_SYSTEM_READ];
    new_ops->file.write = new_driver->desc.functions[DRIVER_FILE_SYSTEM_WRITE];
    new_ops->file.truncate = new_driver->desc.functions[DRIVER_FILE_SYSTEM_TRUNCATE];
    new_ops->file.create = new_driver->desc.functions[DRIVER_FILE_SYSTEM_CREATE];
    new_ops->file.unlink = new_driver->desc.functions[DRIVER_FILE_SYSTEM_UNLINK];
    new_ops->file.fstat = new_driver->desc.functions[DRIVER_FILE_SYSTEM_FSTAT];
    new_ops->file.ioctl = new_driver->desc.functions[DRIVER_FILE_SYSTEM_IOCTL];
    new_ops->file.mmap = new_driver->desc.functions[DRIVER_FILE_SYSTEM_MMAP];

    new_ops->dentry.write_inode = new_driver->desc.functions[DRIVER_FILE_SYSTEM_WRITE_INODE];
    new_ops->dentry.read_inode = new_driver->desc.functions[DRIVER_FILE_SYSTEM_READ_INODE];
    new_ops->dentry.free_inode = new_driver->desc.functions[DRIVER_FILE_SYSTEM_FREE_INODE];
    new_ops->dentry.get_fsdata = new_driver->desc.functions[DRIVER_FILE_SYSTEM_GET_FSDATA];

    fs_desc_t new_fs;
    new_fs.driver = new_driver;
    new_fs.ops = new_ops;
    dynarr_push(&_vfs_fses, &new_fs);
}

int vfs_open(dentry_t* file, file_descriptor_t* fd, uint32_t flags)
{
    thread_t* cur_thread = RUNNING_THREAD;

    if (!file) {
        return -EFAULT;
    }

    if (!fd) {
        return -EFAULT;
    }

    if (dentry_test_flag(file, DENTRY_PRIVATE)) {
        return -EPERM;
    }

    if (dentry_inode_test_flag(file, S_IFDIR) && !(flags & O_DIRECTORY)) {
        return -EISDIR;
    }

    if (flags & O_EXEC) {
        if (vfs_perm_to_execute(file, cur_thread) != 0) {
            log("can't open exec");
            return -EACCES;
        }
    }

    if (flags & O_WRONLY) {
        if (vfs_perm_to_write(file, cur_thread) != 0) {
            log("can't open write");
            return -EACCES;
        }
        if (dentry_inode_test_flag(file, S_IFDIR)) {
            return -EISDIR;
        }
    }

    if (flags & O_RDONLY) {
        if (vfs_perm_to_read(file, cur_thread) != 0) {
            log("can't open read");
            return -EACCES;
        }
    }

    /* If it has custom open, let's use it */
    if (file->ops->file.open) {
        int res = file->ops->file.open(file, fd, flags);
        /* Devfs can't find the right one and returns ENOEXEC in this case. */
        if (res != -ENOEXEC) {
            return res;
        }
    }

    fd->flags = flags;
    fd->type = FD_TYPE_FILE;
    fd->dentry = dentry_duplicate(file);
    fd->offset = 0;
    fd->ops = &file->ops->file;
    lock_init(&fd->lock);
    return 0;
}

static int _int_vfs_do_close(file_descriptor_t* fd)
{
    if (fd->type == FD_TYPE_FILE) {
        dentry_put(fd->dentry);
    } else {
        socket_put(fd->sock_entry);
    }
    fd->dentry = NULL;
    fd->ops = NULL;
    fd->offset = 0;
    return 0;
}

int vfs_close(file_descriptor_t* fd)
{
    if (!fd) {
        return -EFAULT;
    }
    lock_acquire(&fd->lock);
    int res = _int_vfs_do_close(fd);
    lock_release(&fd->lock);
    return res;
}

int vfs_create(dentry_t* dir, const char* name, uint32_t len, mode_t mode, uid_t uid, gid_t gid)
{
    /* Check if there is a file with the same name */
    dentry_t* tmp;
    if (vfs_lookup(dir, name, len, &tmp) == 0) {
        dentry_put(tmp);
        return -EEXIST;
    }

    return dir->ops->file.create(dir, name, len, mode, uid, gid);
}

int vfs_unlink(dentry_t* file)
{
    if (dentry_inode_test_flag(file, S_IFDIR)) {
        return -EPERM;
    }

    if (file->inode->links_count == 1) {
        /* According to docs, we don't delete inode while it's opened somewhere. */
        dentry_set_flag(file, DENTRY_INODE_TO_BE_DELETED);
#ifdef VFS_DEBUG
        log("[VFS] unlink: the file will be deleted");
#endif
    }

    return file->ops->file.unlink(file);
}

int vfs_lookup(dentry_t* dir, const char* name, uint32_t len, dentry_t** result)
{
    if (!dentry_inode_test_flag(dir, S_IFDIR)) {
        return -ENOTDIR;
    }

    if (len == 1) {
        if (name[0] == '.') {
            *result = dentry_duplicate(dir);
            return 0;
        }
    }

    /* If dir is a mount point, vfs should find it's parent by itself */
    if (dentry_test_flag(dir, DENTRY_MOUNTED)) {
#ifdef VFS_DEBUG
        log("[VFS] Lookup for mounted's parents %d %d : %d %d", dir->dev_indx, dir->inode_indx, dir->parent->dev_indx, dir->parent->inode_indx);
#endif
        if (len == 2) {
            if (name[0] == '.' && name[1] == '.') {
                *result = dentry_duplicate(dir->parent);
                return 0;
            }
        }
    }

    if (!dir->ops->file.lookup) {
        return -ENOEXEC;
    }

    int err = dir->ops->file.lookup(dir, name, len, result);
    if (err) {
        return err;
    }

    return 0;
}

bool vfs_can_read(file_descriptor_t* fd)
{
    lock_acquire(&fd->lock);
    bool res = true;
    if (fd->ops->can_read) {
        res = fd->ops->can_read(fd->dentry, fd->offset);
    }
    lock_release(&fd->lock);
    return res;
}

bool vfs_can_write(file_descriptor_t* fd)
{
    lock_acquire(&fd->lock);
    bool res = true;
    if (fd->ops->can_write) {
        res = fd->ops->can_write(fd->dentry, fd->offset);
    }
    lock_release(&fd->lock);
    return res;
}

int vfs_read(file_descriptor_t* fd, void* buf, uint32_t len)
{
    lock_acquire(&fd->lock);
    int read = fd->ops->read(fd->dentry, (uint8_t*)buf, fd->offset, len);
    if (read > 0) {
        fd->offset += read;
    }
    lock_release(&fd->lock);
    return read;
}

int vfs_write(file_descriptor_t* fd, void* buf, uint32_t len)
{
    lock_acquire(&fd->lock);
    int written = fd->ops->write(fd->dentry, (uint8_t*)buf, fd->offset, len);
    if (written > 0) {
        fd->offset += written;
    }

    if (fd->flags & O_TRUNC) {
        if (fd->ops->truncate) {
            fd->ops->truncate(fd->dentry, fd->offset);
        }
    }

    lock_release(&fd->lock);
    return written;
}

/**
 * A caller to vfs_mkdir should garantee that dentry_t* dir is alive.
 */
int vfs_mkdir(dentry_t* dir, const char* name, size_t len, mode_t mode, uid_t uid, gid_t gid)
{
    if (!dentry_inode_test_flag(dir, S_IFDIR)) {
        return -ENOTDIR;
    }
    return dir->ops->file.mkdir(dir, name, len, mode | S_IFDIR, uid, gid);
}

/**
 * A caller to vfs_rmdir should garantee that dentry_t* dir is alive.
 */
int vfs_rmdir(dentry_t* dir)
{
    if (!dentry_inode_test_flag(dir, S_IFDIR)) {
        return -ENOTDIR;
    }
    if (dentry_test_flag(dir, DENTRY_MOUNTPOINT) || dentry_test_flag(dir, DENTRY_MOUNTED) || dir->d_count != 1) {
        return -EBUSY;
    }

    int err = dir->ops->file.rmdir(dir);
    if (!err) {
        log("Rmdir: will be deleted %d", dir->inode_indx);
        dentry_set_flag(dir, DENTRY_INODE_TO_BE_DELETED);
    }
    return err;
}

int vfs_getdents(file_descriptor_t* dir_fd, uint8_t* buf, uint32_t len)
{
    if (!dentry_inode_test_flag(dir_fd->dentry, S_IFDIR)) {
        return -ENOTDIR;
    }
    lock_acquire(&dir_fd->lock);
    int res = dir_fd->ops->getdents(dir_fd->dentry, buf, &dir_fd->offset, len);
    lock_release(&dir_fd->lock);
    return res;
}

int vfs_fstat(file_descriptor_t* fd, fstat_t* stat)
{
    lock_acquire(&fd->lock);
    // Check if we have a custom fstat
    if (fd->ops->fstat) {
        int res = fd->ops->fstat(fd->dentry, stat);
        lock_release(&fd->lock);
        return res;
    }

    // For drives we set MAJOR=0 and MINOR=drive's id.
    stat->dev = MKDEV(0, fd->dentry->dev_indx);
    stat->ino = fd->dentry->inode_indx;
    stat->mode = fd->dentry->inode->mode;
    stat->size = fd->dentry->inode->size;
    // TODO: Fill more stat data here.

    lock_release(&fd->lock);
    return 0;
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
    } else {
        cur_dent = dentry_duplicate(dentry);
    }

    while (*path != '\0') {
        while (*path == '/')
            path++;
        const char* name = path;

        int len = 0;
        while (*path != '\0' && *path != '/')
            path++, len++;

        if (len == 0) {
            break;
        }

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

        // Check for . & .. to not to mess up dentry's parent.
        if (cur_dent != parent_dent && parent_dent->parent != cur_dent) {
            dentry_set_parent(cur_dent, parent_dent);
        }
        dentry_put(parent_dent);
    }

    *result = dentry_duplicate(cur_dent);
    dentry_put(cur_dent);
    return 0;
}

int vfs_resolve_path(const char* path, dentry_t** result)
{
    return vfs_resolve_path_start_from((dentry_t*)NULL, path, result);
}

int vfs_mount(dentry_t* mountpoint, device_t* dev, uint32_t fs_indx)
{
    if (dentry_test_flag(mountpoint, DENTRY_MOUNTPOINT)) {
#ifdef VFS_DEBUG
        log("[VFS] Already a mount point\n");
#endif
        return -EBUSY;
    }
    if (!dentry_inode_test_flag(mountpoint, S_IFDIR)) {
#ifdef VFS_DEBUG
        log("[VFS] Not a dir\n");
#endif
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
    lock_acquire(&mounted_dentry->lock);
    if (!dentry_test_flag_lockless(mounted_dentry, DENTRY_MOUNTED)) {
#ifdef VFS_DEBUG
        log_warn("[VFS] Not mounted\n");
#endif
        lock_release(&mounted_dentry->lock);
        return -EPERM;
    }

    dentry_t* mountpoint = mounted_dentry->mountpoint;

    if (!dentry_test_flag_lockless(mountpoint, DENTRY_MOUNTPOINT)) {
#ifdef VFS_DEBUG
        log_warn("[VFS] Not a mountpoint\n");
#endif
        lock_release(&mounted_dentry->lock);
        return -EPERM;
    }

    dentry_rem_flag_lockless(mounted_dentry, DENTRY_MOUNTED);
    dentry_rem_flag(mountpoint, DENTRY_MOUNTPOINT);

    mounted_dentry->mountpoint = NULL;
    mountpoint->mounted_dentry = NULL;

    dentry_put_lockless(mounted_dentry);
    dentry_put(mountpoint);

    if (dentry_test_flag(mountpoint, DENTRY_MOUNTED)) {
        vfs_umount(mountpoint);
    }

    lock_release(&mounted_dentry->lock);
    return 0;
}

static proc_zone_t* _vfs_do_mmap(file_descriptor_t* fd, mmap_params_t* params)
{
    bool map_shared = ((params->flags & MAP_SHARED) > 0);
    bool map_private = ((params->flags & MAP_PRIVATE) > 0);

    proc_zone_t* zone;

    if (map_private) {
        zone = proc_new_random_zone(RUNNING_THREAD->process, params->size);
        zone->type = ZONE_TYPE_MAPPED_FILE_PRIVATLY;
        zone->file = dentry_duplicate(fd->dentry);
        zone->offset = params->offset;
    } else {
        /* TODO */
        return 0;
    }

    return zone;
}

proc_zone_t* vfs_mmap(file_descriptor_t* fd, mmap_params_t* params)
{
    lock_acquire(&fd->lock);
    /* Check if we have a custom mmap for a dentry */
    if (fd->dentry->ops->file.mmap) {
        proc_zone_t* res = fd->dentry->ops->file.mmap(fd->dentry, params);
        if ((uint32_t)res != VFS_USE_STD_MMAP) {
            lock_release(&fd->lock);
            return res;
        }
    }
    proc_zone_t* res = _vfs_do_mmap(fd, params);
    lock_release(&fd->lock);
    return res;
}

int vfs_munmap(proc_t* p, proc_zone_t* zone)
{
    if (!(zone->flags & ZONE_TYPE_MAPPED_FILE_PRIVATLY) && !(zone->flags & ZONE_TYPE_MAPPED_FILE_SHAREDLY)) {
        return -EFAULT;
    }

    dentry_put(zone->file);

    for (uint32_t vaddr = zone->start; vaddr < zone->start + zone->len + 1; vaddr += VMM_PAGE_SIZE) {
        system_flush_tlb_entry(vaddr);
    }
    proc_delete_zone(p, zone);

    return 0;
}

int vfs_perm_to_read(dentry_t* dentry, thread_t* thread)
{
    // If no running, so call is from kernel
    if (!thread) {
        return 0;
    }

    proc_t* proc = thread->process;
    uid_t uid = proc->uid;
    gid_t gid = proc->gid;

    mode_t mode = dentry->inode->mode;
    uid_t fuid = dentry->inode->uid;
    gid_t fgid = dentry->inode->gid;
    if (proc_is_su(proc)) {
        return 0;
    }
    if (uid == fuid && (mode & S_IRUSR) == S_IRUSR) {
        return 0;
    }
    if (gid == fgid && (mode & S_IRGRP) == S_IRGRP) {
        return 0;
    }
    if (uid != fuid && gid != fgid && (mode & S_IROTH) == S_IROTH) {
        return 0;
    }

    return -EPERM;
}

int vfs_perm_to_write(dentry_t* dentry, thread_t* thread)
{
    // If no running, so call is from kernel
    if (!thread) {
        return 0;
    }

    proc_t* proc = thread->process;
    uid_t uid = proc->uid;
    gid_t gid = proc->gid;

    mode_t mode = dentry->inode->mode;
    uid_t fuid = dentry->inode->uid;
    gid_t fgid = dentry->inode->gid;
    if (proc_is_su(proc)) {
        return 0;
    }
    if (uid == fuid && (mode & S_IWUSR) == S_IWUSR) {
        return 0;
    }
    if (gid == fgid && (mode & S_IWGRP) == S_IWGRP) {
        return 0;
    }
    if (uid != fuid && gid != fgid && (mode & S_IWOTH) == S_IWOTH) {
        return 0;
    }

    return -EPERM;
}

int vfs_perm_to_execute(dentry_t* dentry, thread_t* thread)
{
    // If no running, so call is from kernel
    if (!thread) {
        return 0;
    }

    proc_t* proc = thread->process;
    uid_t uid = proc->uid;
    gid_t gid = proc->gid;

    mode_t mode = dentry->inode->mode;
    uid_t fuid = dentry->inode->uid;
    gid_t fgid = dentry->inode->gid;
    if (proc_is_su(proc)) {
        return 0;
    }
    if (uid == fuid && (mode & S_IXUSR) == S_IXUSR) {
        return 0;
    }
    if (gid == fgid && (mode & S_IXGRP) == S_IXGRP) {
        return 0;
    }
    if (uid != fuid && gid != fgid && (mode & S_IXOTH) == S_IXOTH) {
        return 0;
    }

    return -EPERM;
}
