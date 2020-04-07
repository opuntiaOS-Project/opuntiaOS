#ifndef __oneOS__FS__EXT2__EXT2_H
#define __oneOS__FS__EXT2__EXT2_H

#include <fs/vfs.h>
#include <types.h>

#define SUPERBLOCK_START 1024
#define SUPERBLOCK_LEN (sizeof(superblock_t))
typedef struct {
    uint32_t inodes_count;
    uint32_t blocks_count;
    uint32_t r_blocks_count;
    uint32_t free_blocks_count;
    uint32_t free_inodes_count;
    uint32_t first_data_block;
    uint32_t log_block_size;
    uint32_t log_frag_size;
    uint32_t blocks_per_group;
    uint32_t frags_per_group;
    uint32_t inodes_per_group;
    uint32_t mtime;
    uint32_t wtime;
    uint16_t mnt_count;
    uint16_t max_mnt_count;
    uint16_t magic;
    uint16_t state;
    uint16_t errors;
    uint16_t minor_rev_level;
    uint32_t lastcheck;
    uint32_t checkinterval;
    uint32_t creator_os;
    uint32_t rev_level;
    uint16_t def_resuid;
    uint16_t def_resgid;

    uint32_t first_ino;
    uint16_t inode_size;
    uint16_t block_group_nr;
    uint32_t feature_compat;
    uint32_t feature_incompat;
    uint32_t feature_ro_compat;
    uint8_t  uuid[16];
    uint8_t  volume_name[16];
    uint8_t  last_mounted[64];
    uint32_t algo_bitmap;

    uint8_t  prealloc_blocks;
    uint8_t  prealloc_dir_blocks;

    // current jurnalling is unsupported
    uint8_t unused[1024-206];
} superblock_t;


void ext2_install();
bool ext2_recognize_drive(vfs_device_t *dev);
void ext2_stub();

#endif // __oneOS__FS__EXT2__EXT2_H