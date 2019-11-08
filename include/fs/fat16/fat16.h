#ifndef __oneOS__FS__FAT16__FAT16_H
#define __oneOS__FS__FAT16__FAT16_H

#include <fs/vfs.h>

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

void fat16_install();

bool fat16_recognize(vfs_device_t *t_vfs_dev);

void fat16_create_dir();
void fat16_lookup_dir();
void fat16_remove_dir();

void fat16_write_file();
void fat16_read_file();
void fat16_remove_file();

#endif // __oneOS__FS__FAT16__FAT16_H
