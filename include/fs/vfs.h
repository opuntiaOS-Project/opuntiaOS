#ifndef __oneOS__FS__VFS_H
#define __oneOS__FS__VFS_H

#include <drivers/driver_manager.h>

#define VFS_MAX_FS_COUNT 5
#define VFS_MAX_DEV_COUNT 5
#define VFS_MAX_FILENAME 16
#define VFS_MAX_FILENAME_EXT 4

typedef struct {
    void* recognize;

    void* create_dir;
    void* lookup_dir;
    void* remove_dir;

    void* write_file;
    void* read_file;
    void* remove_file;
} fs_desc_t;

typedef struct {
    int8_t fs;
    uint8_t translate_id;
    device_t dev;
} vfs_device_t;

typedef struct {
    char filename[VFS_MAX_FILENAME];
    char filename_ext[VFS_MAX_FILENAME_EXT];
    uint8_t attributes;
    uint16_t file_size;
} vfs_element_t;

void vfs_install();
void vfs_add_device(device_t t_new_dev);
void vfs_add_fs(fs_desc_t t_new_fs);


// Test Func
// Will be deleted
void vfs_test();

void open();
void close();
void read();
void write();
void create_dir();
void remove_dir();

#endif // __oneOS__FS__VFS_H
