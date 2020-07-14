#include <algo/dynamic_array.h>
#include <fs/devfs/devfs.h>
#include <mem/kmalloc.h>

static dynamic_array_t data;

int devfs_read_inode(dentry_t* dentry)
{
    
    return 0;
}

int devfs_write_inode(dentry_t* dentry)
{
    return 0;
}

int devfs_getdirent(dentry_t* dir, uint32_t* offset, dirent_t* res)
{
    // FIXME: Currently we return error.
    return -1;
}

fsdata_t devfs_data(dentry_t* dentry)
{
    fsdata_t fsdata;
    fsdata.sb = 0;
    fsdata.gt = 0;
    return fsdata;
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
    fs_desc.functions[DRIVER_FILE_SYSTEM_READ] = 0;
    fs_desc.functions[DRIVER_FILE_SYSTEM_WRITE] = 0;
    fs_desc.functions[DRIVER_FILE_SYSTEM_MKDIR] = 0;
    fs_desc.functions[DRIVER_FILE_SYSTEM_EJECT_DEVICE] = 0;

    fs_desc.functions[DRIVER_FILE_SYSTEM_READ_INODE] = devfs_read_inode;
    fs_desc.functions[DRIVER_FILE_SYSTEM_WRITE_INODE] = devfs_write_inode;
    fs_desc.functions[DRIVER_FILE_SYSTEM_GET_FSDATA] = devfs_data;
    fs_desc.functions[DRIVER_FILE_SYSTEM_LOOKUP] = 0;
    fs_desc.functions[DRIVER_FILE_SYSTEM_GETDIRENT] = devfs_getdirent;
    return fs_desc;
}

void devfs_install()
{
    driver_install(_devfs_driver_info());
}

/**
 * Register a device.
 */

devfs_entry_t* devfs_mkdir(devfs_entry_t* dir, const char* name, uint32_t len)
{
    
}

devfs_entry_t* devfs_register(devfs_entry_t* dir, const char* name, uint32_t len, mode_t mode, file_ops_t* handlers)
{
    
}
