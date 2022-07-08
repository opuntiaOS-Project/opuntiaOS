/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
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

static void vfs_recieve_notification(uintptr_t msg, uintptr_t param);
static int _vfs_loadpage_from_mmap_file(struct memzone* zone, uintptr_t vaddr);

static vm_ops_t mmap_file_vm_ops = {
    .load_page_content = _vfs_loadpage_from_mmap_file,
    .restore_swapped_page = NULL,
    .swap_page_mode = NULL,
};

driver_desc_t _vfs_driver_info()
{
    driver_desc_t vfs_desc = { 0 };
    vfs_desc.type = DRIVER_VIRTUAL_FILE_SYSTEM;
    vfs_desc.listened_device_mask = DEVICE_STORAGE;
    vfs_desc.listened_driver_mask = DRIVER_FILE_SYSTEM;

    vfs_desc.system_funcs.recieve_notification = vfs_recieve_notification;

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
    devman_register_driver(_vfs_driver_info(), "vfs");
    dynarr_init_of_size(fs_desc_t, &_vfs_fses, MAX_FS);
}
devman_register_driver_installation(vfs_install);

static void vfs_recieve_notification(uintptr_t msg, uintptr_t param)
{
    switch (msg) {
    case DEVMAN_NOTIFICATION_NEW_DEVICE:
        return (void)vfs_add_dev((device_t*)param);
    case DEVMAN_NOTIFICATION_NEW_DRIVER:
        return (void)vfs_add_fs((driver_t*)param);
    default:
        break;
    }
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
    spinlock_init(&_vfs_devices[dev->id].fslock);
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

int vfs_add_fs(driver_t* new_driver)
{
    if (new_driver->desc.type != DRIVER_FILE_SYSTEM) {
        return -1;
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
    new_ops->file.fchmod = new_driver->desc.functions[DRIVER_FILE_SYSTEM_FCHMOD];
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
    return 0;
}

int vfs_check_open_perms(const path_t* path, int flags)
{
    dentry_t* file = path->dentry;
    thread_t* cur_thread = RUNNING_THREAD;

    if (TEST_FLAG(flags, O_EXEC)) {
        if (vfs_perm_to_execute(file, cur_thread) != 0) {
#ifdef VFS_DEBUG
            log("Can't open exec");
#endif
            return -EACCES;
        }
    }

    if (TEST_FLAG(flags, O_WRONLY)) {
        if (vfs_perm_to_write(file, cur_thread) != 0) {
#ifdef VFS_DEBUG
            log("Can't open write");
#endif
            return -EACCES;
        }
        if (dentry_test_mode(file, S_IFDIR)) {
            return -EISDIR;
        }
    }

    if (TEST_FLAG(flags, O_RDONLY)) {
        if (vfs_perm_to_read(file, cur_thread) != 0) {
#ifdef VFS_DEBUG
            log("Can't open read");
#endif
            return -EACCES;
        }
    }

    return 0;
}

int vfs_open(const path_t* path, file_descriptor_t* fd, int flags)
{
    if (!path_is_valid(path)) {
        return -EINVAL;
    }

    dentry_t* file = path->dentry;
    if (!fd) {
        return -EFAULT;
    }

    if (dentry_test_flag(file, DENTRY_PRIVATE)) {
        return -EPERM;
    }

    if (dentry_test_mode(file, S_IFDIR) && !TEST_FLAG(flags, O_DIRECTORY)) {
        return -EISDIR;
    }

    int err = vfs_check_open_perms(path, flags);
    if (err) {
        return err;
    }

    // If it has custom open, let's use it.
    if (file->ops->file.open) {
        int res = file->ops->file.open(path, fd, flags);
        // FS can't find the right one and returns ENOEXEC in this case.
        if (res != -ENOEXEC) {
            return res;
        }
    }

    fd->flags = flags;
    fd->file = file_init_path(path);
    fd->offset = 0;
    return 0;
}

int vfs_close(file_descriptor_t* fd)
{
    if (!fd) {
        return -EFAULT;
    }
    file_put(fd->file);
    fd->file = NULL;
    fd->offset = 0;
    fd->flags = 0;
    return 0;
}

int vfs_create(const path_t* path, const char* name, size_t len, mode_t mode, uid_t uid, gid_t gid)
{
    if (!path_is_valid(path)) {
        return -EINVAL;
    }

    dentry_t* dir = path->dentry;
    if (!dentry_test_mode(dir, S_IFDIR)) {
        return -ENOTDIR;
    }

    // Check if there is a file with the same name
    path_t tmp_path;
    if (vfs_lookup(path, name, len, &tmp_path) == 0) {
        path_put(&tmp_path);
        return -EEXIST;
    }

    if (!dir->ops->file.create) {
        return -EROFS;
    }

    // Turn mode into a regular file if mode is not set.
    if (!S_ISSOCK(mode) && !S_ISFIFO(mode) && !S_ISBLK(mode) && !S_ISCHR(mode)) {
        mode |= S_IFREG;
    }

    return dir->ops->file.create(path, name, len, mode, uid, gid);
}

int vfs_unlink(const path_t* filepath)
{
    if (!path_is_valid(filepath)) {
        return -EINVAL;
    }

    dentry_t* file = filepath->dentry;
    if (dentry_test_mode(file, S_IFDIR)) {
        return -EPERM;
    }

    if (file->inode->links_count == 1) {
        /* According to docs, we don't delete inode while it's opened somewhere. */
        dentry_set_flag(file, DENTRY_INODE_TO_BE_DELETED);
#ifdef VFS_DEBUG
        log("[VFS] unlink: the file will be deleted");
#endif
    }

    if (!file->ops->file.unlink) {
        return -EROFS;
    }
    return file->ops->file.unlink(filepath);
}

int vfs_lookup(const path_t* path, const char* name, size_t len, path_t* result)
{
    if (!path_is_valid(path)) {
        return -EINVAL;
    }

    dentry_t* dir = path->dentry;
    if (!dentry_test_mode(dir, S_IFDIR)) {
        return -ENOTDIR;
    }

    if (len == 1) {
        if (name[0] == '.') {
            result->dentry = dentry_duplicate(dir);
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
                result->dentry = dentry_duplicate(dir->parent);
                return 0;
            }
        }
    }

    if (!dir->ops->file.lookup) {
        return -ENOEXEC;
    }

    int err = dir->ops->file.lookup(path, name, len, result);
    if (err) {
        return err;
    }

    return 0;
}

bool vfs_can_read(file_descriptor_t* fd)
{
    spinlock_acquire(&fd->file->lock);
    bool res = true;
    if (fd->file->ops->can_read) {
        res = fd->file->ops->can_read(fd->file, fd->offset);
    }
    spinlock_release(&fd->file->lock);
    return res;
}

bool vfs_can_write(file_descriptor_t* fd)
{
    spinlock_acquire(&fd->file->lock);
    bool res = true;
    if (fd->file->ops->can_write) {
        res = fd->file->ops->can_write(fd->file, fd->offset);
    }
    spinlock_release(&fd->file->lock);
    return res;
}

int vfs_read(file_descriptor_t* fd, void __user* buf, size_t len)
{
    spinlock_acquire(&fd->file->lock);
    if (!fd->file->ops->read) {
        spinlock_release(&fd->file->lock);
        return -ENOEXEC;
    }

    int read = fd->file->ops->read(fd->file, (uint8_t __user*)buf, fd->offset, len);
    if (read > 0) {
        fd->offset += read;
    }
    spinlock_release(&fd->file->lock);
    return read;
}

int vfs_write(file_descriptor_t* fd, void __user* buf, size_t len)
{
    spinlock_acquire(&fd->file->lock);
    if (!fd->file->ops->write) {
        spinlock_release(&fd->file->lock);
        return -EROFS;
    }

    int written = fd->file->ops->write(fd->file, (uint8_t __user*)buf, fd->offset, len);
    if (written > 0) {
        fd->offset += written;
    }

    if (TEST_FLAG(fd->flags, O_TRUNC)) {
        if (fd->file->ops->truncate) {
            fd->file->ops->truncate(fd->file, fd->offset);
        }
    }

    spinlock_release(&fd->file->lock);
    return written;
}

int vfs_mkdir(const path_t* dirpath, const char* name, size_t len, mode_t mode, uid_t uid, gid_t gid)
{
    if (!path_is_valid(dirpath)) {
        return -EINVAL;
    }

    dentry_t* dir = dirpath->dentry;
    if (!dentry_test_mode(dir, S_IFDIR)) {
        return -ENOTDIR;
    }

    if (!dir->ops->file.mkdir) {
        return -EROFS;
    }
    return dir->ops->file.mkdir(dirpath, name, len, mode | S_IFDIR, uid, gid);
}

int vfs_rmdir(const path_t* dirpath)
{
    if (!path_is_valid(dirpath)) {
        return -EINVAL;
    }

    dentry_t* dir = dirpath->dentry;
    if (!dentry_test_mode(dir, S_IFDIR)) {
        return -ENOTDIR;
    }

    if (dentry_test_flag(dir, DENTRY_MOUNTPOINT) || dentry_test_flag(dir, DENTRY_MOUNTED) || dir->d_count != 1) {
        return -EBUSY;
    }

    if (!dir->ops->file.rmdir) {
        return -EROFS;
    }

    int err = dir->ops->file.rmdir(dirpath);
    if (!err) {
#ifdef VFS_DEBUG
        log("Rmdir: will be deleted %d", dir->inode_indx);
#endif
        dentry_set_flag(dir, DENTRY_INODE_TO_BE_DELETED);
    }
    return err;
}

int vfs_getdents(file_descriptor_t* fd, void __user* buf, size_t len)
{
    dentry_t* dir_dentry = file_dentry_assert(fd->file);
    if (!dir_dentry) {
        return -EINVAL;
    }

    if (!dentry_test_mode(dir_dentry, S_IFDIR)) {
        return -ENOTDIR;
    }
    spinlock_acquire(&fd->file->lock);
    ASSERT(fd->file->ops->getdents && "FS marked file as dir, but no getdents impl");
    int res = fd->file->ops->getdents(dir_dentry, buf, &fd->offset, len);
    spinlock_release(&fd->file->lock);
    return res;
}

int vfs_fstat(file_descriptor_t* fd, stat_t* stat)
{
    if (fd->file->ops->fstat) {
        int res = fd->file->ops->fstat(fd->file, stat);
        return res;
    }

    spinlock_acquire(&fd->file->lock);
    dentry_t* dentry = file_dentry(fd->file);

    switch (fd->file->type) {
    case FTYPE_FILE:
        ASSERT(dentry);
        // For drives we set MAJOR=0 and MINOR=drive's id.
        stat->st_dev = MKDEV(0, dentry->dev_indx);
        stat->st_ino = dentry->inode_indx;
        stat->st_mode = dentry->inode->mode;
        stat->st_size = dentry->inode->size;
        stat->st_uid = dentry->inode->uid;
        stat->st_gid = dentry->inode->gid;
        stat->st_blksize = dentry->fsdata.blksize;
        stat->st_nlink = dentry->inode->links_count;
        stat->st_blocks = dentry->inode->blocks;
        stat->st_atim.tv_sec = dentry->inode->atime;
        stat->st_atim.tv_nsec = 0;
        stat->st_mtim.tv_sec = dentry->inode->mtime;
        stat->st_mtim.tv_nsec = 0;
        stat->st_ctim.tv_sec = dentry->inode->ctime;
        stat->st_ctim.tv_nsec = 0;
        break;

    case FTYPE_SOCKET:
        ASSERT(false && "No fstat for FD_TYPE_SOCKET");
        break;

    default:
        break;
    }

    spinlock_release(&fd->file->lock);
    return 0;
}

int vfs_chmod(const path_t* vfspath, mode_t mode)
{
    if (!path_is_valid(vfspath)) {
        return -EINVAL;
    }

    proc_t* current_p = RUNNING_THREAD->process;
    dentry_t* dentry = vfspath->dentry;
    if (dentry->inode->uid != current_p->euid && !proc_is_su(current_p)) {
        return -EPERM;
    }

    // TODO: Check if FS is readonly.
    dentry_set_flag(dentry, DENTRY_DIRTY);
    dentry->inode->mode = (dentry->inode->mode & ~(uint32_t)07777) | (mode & (uint32_t)07777);
    return 0;
}

int vfs_fchmod(file_descriptor_t* fd, mode_t mode)
{
    spinlock_acquire(&fd->file->lock);
    if (!fd->file->ops->fchmod) {
        spinlock_release(&fd->file->lock);
        return -EROFS;
    }

    int err = fd->file->ops->fchmod(fd->file, mode);
    spinlock_release(&fd->file->lock);
    return err;
}

int vfs_resolve_path_start_from(const path_t* vfspath, const char* path, path_t* result)
{
    path_t intpath;
    dentry_t* cur_dent;

    if (!vfspath || !vfspath->dentry || path[0] == '/') {
        ASSERT(root_fs_dev_id != -1);
        cur_dent = dentry_get(root_fs_dev_id, 2);
        while (*path == '/') {
            path++;
        }
    } else {
        cur_dent = dentry_duplicate(vfspath->dentry);
    }

    while (*path != '\0') {
        while (*path == '/') {
            path++;
        }
        const char* name = path;

        int len = 0;
        while (*path != '\0' && *path != '/') {
            path++, len++;
        }

        if (len == 0) {
            break;
        }

        dentry_t* parent_dent = cur_dent;
        intpath.dentry = cur_dent;
        if (vfs_lookup(&intpath, name, len, &intpath) < 0) {
            return -ENOENT;
        }
        cur_dent = intpath.dentry;

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
            char* newname = kmalloc(len + 1);
            memcpy(newname, name, len);
            newname[len] = '\0';
            dentry_set_filename(cur_dent, newname);
            dentry_set_parent(cur_dent, parent_dent);
        }
        dentry_put(parent_dent);
    }

    result->dentry = dentry_duplicate(cur_dent);
    dentry_put(cur_dent);
    return 0;
}

int vfs_resolve_path(const char* path, path_t* result)
{
    return vfs_resolve_path_start_from((path_t*)NULL, path, result);
}

bool _vfs_is_root_dentry(dentry_t* dent)
{
    return dent && dent->dev_indx == root_fs_dev_id && dent->inode_indx == 2;
}

int _vfs_len_of_absolute_path(const path_t* path)
{
    int path_len = 0;
    if (!path_is_valid(path)) {
        return 0;
    }

    dentry_t* dent = path->dentry;
    if (_vfs_is_root_dentry(dent)) {
        return 2;
    }

    while (!_vfs_is_root_dentry(dent) && dent->parent) {
        path_len += strlen(dent->filename) + 1;
        dent = dent->parent;
    }

    if (!_vfs_is_root_dentry(dent)) {
        return -EAGAIN;
    }

    return path_len + 1;
}

int vfs_get_absolute_path(const path_t* path, char* buf, int len)
{
    if (!path_is_valid(path)) {
        return 0;
    }

    dentry_t* dent = path->dentry;
    int req_len = _vfs_len_of_absolute_path(path);
    if (req_len < 0 || !buf) {
        return req_len;
    }

    if (req_len > len) {
        return -EOVERFLOW;
    }

    if (_vfs_is_root_dentry(dent)) {
        buf[0] = '/';
        buf[1] = '\0';
        return req_len;
    }

    char* start = &buf[req_len - 1];
    while (!_vfs_is_root_dentry(dent) && dent->parent) {
        int len = strlen(dent->filename);
        start -= len;
        memcpy(start, dent->filename, len);
        start -= 1;
        *start = '/';
        dent = dent->parent;
    }
    buf[req_len - 1] = '\0';

    return req_len;
}

int vfs_mount(path_t* mount_path, device_t* dev, uint32_t fs_indx)
{
    dentry_t* mountpoint = mount_path->dentry;
    if (!mountpoint) {
        return -EINVAL;
    }

    if (dentry_test_flag(mountpoint, DENTRY_MOUNTPOINT)) {
#ifdef VFS_DEBUG
        log("[VFS] Already a mount point\n");
#endif
        return -EBUSY;
    }
    if (!dentry_test_mode(mountpoint, S_IFDIR)) {
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
    spinlock_acquire(&mounted_dentry->lock);
    if (!dentry_test_flag_locked(mounted_dentry, DENTRY_MOUNTED)) {
#ifdef VFS_DEBUG
        log_warn("[VFS] Not mounted\n");
#endif
        spinlock_release(&mounted_dentry->lock);
        return -EPERM;
    }

    dentry_t* mountpoint = mounted_dentry->mountpoint;

    if (!dentry_test_flag_locked(mountpoint, DENTRY_MOUNTPOINT)) {
#ifdef VFS_DEBUG
        log_warn("[VFS] Not a mountpoint\n");
#endif
        spinlock_release(&mounted_dentry->lock);
        return -EPERM;
    }

    dentry_rem_flag_locked(mounted_dentry, DENTRY_MOUNTED);
    dentry_rem_flag(mountpoint, DENTRY_MOUNTPOINT);

    mounted_dentry->mountpoint = NULL;
    mountpoint->mounted_dentry = NULL;

    dentry_put_locked(mounted_dentry);
    dentry_put(mountpoint);

    if (dentry_test_flag(mountpoint, DENTRY_MOUNTED)) {
        vfs_umount(mountpoint);
    }

    spinlock_release(&mounted_dentry->lock);
    return 0;
}

static int _vfs_loadpage_from_mmap_file(struct memzone* zone, uintptr_t vaddr)
{
    ASSERT(zone->type & ZONE_TYPE_MAPPED_FILE_PRIVATLY);

    // Could ignore file_size here, because in case of EOF file_ops->read() stops reading.
    size_t offset = zone->file_offset + (PAGE_START(vaddr) - zone->vaddr);
    zone->file->ops->read(zone->file, (void*)PAGE_START(vaddr), offset, VMM_PAGE_SIZE);
    return 0;
}

static memzone_t* _vfs_do_mmap(file_descriptor_t* fd, mmap_params_t* params)
{
    dentry_t* dentry = file_dentry_assert(fd->file);
    bool map_shared = TEST_FLAG(params->flags, MAP_SHARED);
    bool map_private = TEST_FLAG(params->flags, MAP_PRIVATE);

    memzone_t* zone = NULL;

    if (map_private) {
        spinlock_acquire(&fd->file->lock);
        zone = memzone_new_random(RUNNING_THREAD->process->address_space, params->size);
        zone->type = ZONE_TYPE_MAPPED_FILE_PRIVATLY;
        zone->file = file_duplicate_locked(fd->file);
        zone->file_offset = params->offset;
        zone->file_size = dentry->inode->size;
        zone->ops = &mmap_file_vm_ops;
        spinlock_release(&fd->file->lock);
    }

    return zone;
}

memzone_t* vfs_mmap(file_descriptor_t* fd, mmap_params_t* params)
{
    // Check if we have a custom mmap for a dentry.
    if (fd->file->ops->mmap) {
        memzone_t* res = fd->file->ops->mmap(fd->file, params);
        if ((uintptr_t)res != VFS_USE_STD_MMAP) {
            return res;
        }
    }
    memzone_t* res = _vfs_do_mmap(fd, params);
    return res;
}

int vfs_munmap(proc_t* p, memzone_t* zone)
{
    if (!TEST_FLAG(zone->mmu_flags, ZONE_TYPE_MAPPED_FILE_PRIVATLY) && !TEST_FLAG(zone->mmu_flags, ZONE_TYPE_MAPPED_FILE_SHAREDLY)) {
        return -EFAULT;
    }

    file_put(zone->file);

    for (uintptr_t vaddr = zone->vaddr; vaddr < zone->vaddr + zone->len + 1; vaddr += VMM_PAGE_SIZE) {
        system_flush_local_tlb_entry(vaddr);
    }
    memzone_free(p->address_space, zone);

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
    if (uid == fuid && TEST_FLAG(mode, S_IRUSR)) {
        return 0;
    }
    if (gid == fgid && TEST_FLAG(mode, S_IRGRP)) {
        return 0;
    }
    if (uid != fuid && gid != fgid && TEST_FLAG(mode, S_IROTH)) {
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
    if (uid == fuid && TEST_FLAG(mode, S_IWUSR)) {
        return 0;
    }
    if (gid == fgid && TEST_FLAG(mode, S_IWGRP)) {
        return 0;
    }
    if (uid != fuid && gid != fgid && TEST_FLAG(mode, S_IWOTH)) {
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
    if (uid == fuid && TEST_FLAG(mode, S_IXUSR)) {
        return 0;
    }
    if (gid == fgid && TEST_FLAG(mode, S_IXGRP)) {
        return 0;
    }
    if (uid != fuid && gid != fgid && TEST_FLAG(mode, S_IXOTH)) {
        return 0;
    }

    return -EPERM;
}
