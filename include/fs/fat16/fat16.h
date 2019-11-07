#ifndef __oneOS__FS__FAT16__FAT16_H
#define __oneOS__FS__FAT16__FAT16_H

#include <fs/vfs.h>

void fat16_install();
bool fat16_recognize(vfs_device_t *t_vfs_dev);

#endif // __oneOS__FS__FAT16__FAT16_H
