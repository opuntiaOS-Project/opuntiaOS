#ifndef __oneOS__FS__FAT16__FAT16_H
#define __oneOS__FS__FAT16__FAT16_H

#include <fs/vfs.h>

void fat16_install();

bool fat16_recognize(vfs_device_t *t_vfs_dev);

void fat16_create_dir();
void fat16_lookup_dir();
void fat16_remove_dir();

void fat16_write_file();
void fat16_read_file();
void fat16_remove_file();

#endif // __oneOS__FS__FAT16__FAT16_H
