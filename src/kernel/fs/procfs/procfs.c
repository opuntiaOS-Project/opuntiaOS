#include <fs/vfs.h>
#include <mem/kmalloc.h>

static int test_proc_num = 8;

int procfs_read_inode(dentry_t* dentry)
{
    dentry->inode->mode = S_IFDIR;
    return 0;
}

int procfs_write_inode(dentry_t* dentry)
{
    return 0;
}

int procfs_free_inode(dentry_t* dentry)
{
    return 0;
}

fsdata_t procfs_data(dentry_t* dentry)
{
    fsdata_t fsdata;
    fsdata.sb = 0;
    fsdata.gt = 0;
    return fsdata;
}

/* FIXME: New api */
int procfs_getdents(dentry_t* dir, uint32_t* offset, dirent_t* res)
{
    // if (*offset >= test_proc_num) {
    //     return -1;
    // }

    // res->inode = *offset + 2;
    // char val = (*offset + (int)'0');
    // memcpy(res->name, "Proc", 4);
    // memcpy(res->name+4, &val, 1);

    // (*offset)++;
    return 0;
}

driver_desc_t _procfs_driver_info()
{
    driver_desc_t fs_desc;
    fs_desc.type = DRIVER_FILE_SYSTEM;
    fs_desc.auto_start = false;
    fs_desc.is_device_driver = false;
    fs_desc.is_device_needed = false;
    fs_desc.is_driver_needed = false;
    fs_desc.functions[DRIVER_NOTIFICATION] = 0;
    fs_desc.functions[DRIVER_FILE_SYSTEM_RECOGNIZE] = 0;
    fs_desc.functions[DRIVER_FILE_SYSTEM_PREPARE_FS] = 0;
    fs_desc.functions[DRIVER_FILE_SYSTEM_OPEN] = 0; /* No custom open, vfs will use its code */
    fs_desc.functions[DRIVER_FILE_SYSTEM_READ] = 0;
    fs_desc.functions[DRIVER_FILE_SYSTEM_WRITE] = 0;
    fs_desc.functions[DRIVER_FILE_SYSTEM_TRUNCATE] = 0;
    fs_desc.functions[DRIVER_FILE_SYSTEM_MKDIR] = 0;
    fs_desc.functions[DRIVER_FILE_SYSTEM_EJECT_DEVICE] = 0;

    fs_desc.functions[DRIVER_FILE_SYSTEM_READ_INODE] = procfs_read_inode;
    fs_desc.functions[DRIVER_FILE_SYSTEM_WRITE_INODE] = procfs_write_inode;
    fs_desc.functions[DRIVER_FILE_SYSTEM_FREE_INODE] = procfs_free_inode;
    fs_desc.functions[DRIVER_FILE_SYSTEM_GET_FSDATA] = procfs_data;
    fs_desc.functions[DRIVER_FILE_SYSTEM_LOOKUP] = 0;
    fs_desc.functions[DRIVER_FILE_SYSTEM_GETDENTS] = procfs_getdents;

    fs_desc.functions[DRIVER_FILE_SYSTEM_IOCTL] = 0;
    fs_desc.functions[DRIVER_FILE_SYSTEM_MMAP] = 0;
    return fs_desc;
}

void procfs_install()
{
    driver_install(_procfs_driver_info(), "procfs");
}