#ifndef __oneOS__FS__FAT16__FAT16_H
#define __oneOS__FS__FAT16__FAT16_H

#include <fs/vfs.h>

#define FAT16_ELEMENT_FOLDER 0x10
#define FAT16_ELEMENT_ROOT_FOLDER 0x11
#define FAT16_ELEMENT_NULL 0xFF
#define FAT16_ELEMENT_ROOT_BLOCK_ID 0
#define FAT16_DELETED_SIGN 0xe5
#define FAT16_MAX_FILENAME 8
#define FAT16_MAX_FILENAME_EXT 3

typedef struct {
    uint16_t bytes_per_sector;
    uint16_t bytes_per_cluster;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint32_t start_of_fats;
    uint8_t number_of_fats;
    uint16_t sectors_per_fat;
    uint16_t root_entries;
    uint32_t root_dir_start;
    uint32_t data_seg_start;
    uint8_t* fat_ptr;
} fat16_drive_desc_t;

typedef struct {
    char filename[FAT16_MAX_FILENAME];
    char filename_ext[FAT16_MAX_FILENAME_EXT];
    uint8_t attributes;
    uint16_t start_cluster_id;
    uint16_t file_size;
} fat16_element_t;

void fat16_install();

bool fat16_recognize(vfs_device_t *t_vfs_dev);

bool fat16_create_dir(vfs_device_t *t_vfs_dev, const char *t_path, const char *t_dir_name);
uint32_t fat16_lookup_dir(vfs_device_t *t_vfs_dev, const char *t_path, vfs_element_t* t_buf);
void fat16_remove_dir();

bool fat16_write_file(vfs_device_t *t_vfs_dev, const char *t_path, const char *t_file_name, const char *t_file_ext, const uint8_t *t_data, uint32_t t_size);
uint8_t* fat16_read_file(vfs_device_t *t_vfs_dev, const char *t_path, const char *t_file_name, const char *t_file_ext, uint16_t t_offset, int16_t t_len);
void fat16_remove_file();

#endif // __oneOS__FS__FAT16__FAT16_H
