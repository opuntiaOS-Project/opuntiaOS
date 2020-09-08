/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <errno.h>
#include <fs/vfs.h>
#include <mem/kmalloc.h>
#include <time/time_manager.h>
#include <utils/kassert.h>

#define MAX_BLOCK_LEN 1024

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define SUPERBLOCK _ext2_superblocks[dev->dev->id]
#define GROUP_TABLES _ext2_group_table_info[dev->dev->id].table
#define BLOCK_LEN(sb) (1024 << (sb->log_block_size))
#define TO_EXT_BLOCKS_CNT(sb, x) (x / (2 << (sb->log_block_size)))
#define NORM_FILENAME(x) (x + ((4 - (x & 0b11)) & 0b11))

static superblock_t* _ext2_superblocks[MAX_DEVICES_COUNT];
static groups_info_t _ext2_group_table_info[MAX_DEVICES_COUNT];

driver_desc_t _ext2_driver_info();

/* DRIVE RELATED FUNCTIONS */
static void _ext2_read_from_dev(vfs_device_t* dev, uint8_t* buf, uint32_t start, uint32_t len);
static void _ext2_write_to_dev(vfs_device_t* dev, uint8_t* buf, uint32_t start, uint32_t len);
static uint32_t _ext2_get_disk_size(vfs_device_t* dev);

/* UTILS */
static inline bool _ext2_bitmap_get(uint8_t* bitmap, uint32_t index);
static inline void _ext2_bitmap_set_bit(uint8_t* bitmap, uint32_t index);
static inline void _ext2_bitmap_unset_bit(uint8_t* bitmap, uint32_t index);

/* GROUPS FUNCTIONS */
static inline uint32_t _ext2_get_group_len(superblock_t* sb);
static inline int _ext2_get_groups_cnt(vfs_device_t* dev, superblock_t* sb);

/* BLOCK FUNCTIONS */
static uint32_t _ext2_get_block_offset(superblock_t* sb, uint32_t block_index);

static uint32_t _ext2_get_block_of_inode_lev0(dentry_t* dentry, uint32_t cur_block, uint32_t inode_block_index);
static uint32_t _ext2_get_block_of_inode_lev1(dentry_t* dentry, uint32_t cur_block, uint32_t inode_block_index);
static uint32_t _ext2_get_block_of_inode_lev2(dentry_t* dentry, uint32_t cur_block, uint32_t inode_block_index);
static uint32_t _ext2_get_block_of_inode(dentry_t* dentry, uint32_t inode_block_index);

static int _ext2_set_block_of_inode_lev0(dentry_t* dentry, uint32_t cur_block, uint32_t inode_block_index, uint32_t val);
static int _ext2_set_block_of_inode_lev1(dentry_t* dentry, uint32_t cur_block, uint32_t inode_block_index, uint32_t val);
static int _ext2_set_block_of_inode_lev2(dentry_t* dentry, uint32_t cur_block, uint32_t inode_block_index, uint32_t val);
static int _ext2_set_block_of_inode(dentry_t* dentry, uint32_t inode_block_index, uint32_t val);

static int _ext2_find_free_block_index(vfs_device_t* dev, fsdata_t fsdata, uint32_t* block_index, uint32_t group_index);
static int _ext2_allocate_block_index(vfs_device_t* dev, fsdata_t fsdata, uint32_t* block_index, uint32_t pref_group);
static int _ext2_free_block_index(vfs_device_t* dev, fsdata_t fsdata, uint32_t block_index);

static int _ext2_allocate_block_for_inode(dentry_t* dentry, uint32_t pref_group, uint32_t* block_index);

/* INODE FUNCTIONS */
int ext2_read_inode(dentry_t* dentry);
int ext2_write_inode(dentry_t* dentry);

static int _ext2_find_free_inode_index(vfs_device_t* dev, fsdata_t fsdata, uint32_t* inode_index, uint32_t group_index);
static int _ext2_allocate_inode_index(vfs_device_t* dev, fsdata_t fsdata, uint32_t* inode_index, uint32_t pref_group);
static int _ext2_free_inode_index(vfs_device_t* dev, fsdata_t fsdata, uint32_t inode_index);
static int _ext2_free_inode(dentry_t* dentry);

/* DIR FUNCTIONS */
static int _ext2_lookup_block(vfs_device_t* dev, fsdata_t fsdata, uint32_t block_index, const char* name, uint32_t len, uint32_t* found_inode_index);
static int _ext2_getdirent_block(vfs_device_t* dev, fsdata_t fsdata, uint32_t block_index, uint32_t* offset, dirent_t* dirent);
static int _ext2_add_first_entry_to_dir_block(vfs_device_t* dev, fsdata_t fsdata, uint32_t block_index, dentry_t* child_dentry, const char* filename, uint32_t len);
static int _ext2_add_to_dir_block(vfs_device_t* dev, fsdata_t fsdata, uint32_t block_index, dentry_t* child_dentry, const char* filename, uint32_t len);
static int _ext2_rm_from_dir_block(vfs_device_t* dev, fsdata_t fsdata, uint32_t block_index, dentry_t* child_dentry);

static int _ext2_add_child(dentry_t* dir, dentry_t* child_dentry, const char* name, int len);
static int _ext2_rm_child(dentry_t* dir, dentry_t* child_dentry);
static int _ext2_setup_dir(dentry_t* dir, dentry_t* parent_dir, mode_t mode);

/* FILE FUNCTIONS */
static int _ext2_setup_file(dentry_t* file, mode_t mode);

/* API FUNTIONS */
bool ext2_recognize_drive(vfs_device_t* dev);
int ext2_prepare_fs(vfs_device_t* dev);
void ext2_save_state(vfs_device_t* dev);
fsdata_t get_fsdata(dentry_t* dentry);

int ext2_read(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len);
int ext2_write(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len);
int ext2_lookup(dentry_t* dir, const char* name, uint32_t len, uint32_t* res_inode_indx);
int ext2_mkdir(dentry_t* dir, const char* name, uint32_t len, mode_t mode);
int ext2_getdirent(dentry_t* dir, uint32_t* offset, dirent_t* res);
int ext2_create(dentry_t* dir, const char* name, uint32_t len, mode_t mode);
int ext2_rm(dentry_t* dentry);

/**
 * DRIVE RELATED FUNCTIONS
 */

static void _ext2_read_from_dev(vfs_device_t* dev, uint8_t* buf, uint32_t start, uint32_t len)
{
    void (*read)(device_t * d, uint32_t s, uint8_t * r) = drivers[dev->dev->driver_id].driver_desc.functions[DRIVER_STORAGE_READ];
    int already_read = 0;
    uint32_t sector = start / 512;
    uint32_t start_offset = start % 512;
    uint8_t tmp_buf[512];

    while (len) {
        read(dev->dev, sector, tmp_buf);
        for (int i = 0; i < MIN(512 - start_offset, len); i++) {
            buf[already_read++] = tmp_buf[start_offset + i];
        }
        len -= MIN(512 - start_offset, len);
        sector++;
        start_offset = 0;
    }
}

static void _ext2_write_to_dev(vfs_device_t* dev, uint8_t* buf, uint32_t start, uint32_t len)
{
    void (*read)(device_t * d, uint32_t s, uint8_t * r) = drivers[dev->dev->driver_id].driver_desc.functions[DRIVER_STORAGE_READ];
    void (*write)(device_t * d, uint32_t s, uint8_t * r, uint32_t siz) = drivers[dev->dev->driver_id].driver_desc.functions[DRIVER_STORAGE_WRITE];
    int already_written = 0;
    uint32_t sector = start / 512;
    uint32_t start_offset = start % 512;
    uint8_t tmp_buf[512];
    while (len != 0) {
        if (start_offset != 0 || len < 512) {
            read(dev->dev, sector, tmp_buf);
        }
        for (int i = 0; i < MIN(512 - start_offset, len); i++) {
            tmp_buf[start_offset + i] = buf[already_written++];
        }
        write(dev->dev, sector, tmp_buf, 512);
        len -= MIN(512 - start_offset, len);
        sector++;
        start_offset = 0;
    }
}

static uint32_t _ext2_get_disk_size(vfs_device_t* dev)
{
    uint32_t (*get_size)(device_t * d) = drivers[dev->dev->driver_id].driver_desc.functions[DRIVER_STORAGE_CAPACITY];
    return get_size(dev->dev);
}

/**
 * UTILS
 */

static inline bool _ext2_bitmap_get(uint8_t* bitmap, uint32_t index)
{
    return (bitmap[index / 8] >> (index % 8)) & 1;
}

static inline void _ext2_bitmap_set_bit(uint8_t* bitmap, uint32_t index)
{
    bitmap[index / 8] |= (uint8_t)(1 << (index % 8));
}

static inline void _ext2_bitmap_unset_bit(uint8_t* bitmap, uint32_t index)
{
    bitmap[index / 8] &= ~(1 << (index % 8));
}

/**
 * GROUPS FUNCTIONS
 */

static inline uint32_t _ext2_get_group_len(superblock_t* sb)
{
    return BLOCK_LEN(sb) * BLOCK_LEN(sb) * 8;
}

static inline int _ext2_get_groups_cnt(vfs_device_t* dev, superblock_t* sb)
{
    uint32_t sz = _ext2_get_disk_size(dev) - SUPERBLOCK_START;
    uint32_t ans = sz / _ext2_get_group_len(sb);
    /* TODO: Work with the last smaller group. */
    // if (sz % get_group_len()) {
    //     ans++;
    // }
    return ans;
}

/**
 * BLOCK FUNCTIONS
 */

static uint32_t _ext2_get_block_offset(superblock_t* sb, uint32_t block_index)
{
    return SUPERBLOCK_START + (block_index - 1) * BLOCK_LEN(sb);
}

static uint32_t _ext2_get_block_of_inode_lev0(dentry_t* dentry, uint32_t cur_block, uint32_t inode_block_index)
{
    uint32_t offset = inode_block_index;
    uint32_t res;
    _ext2_read_from_dev(dentry->dev, (uint8_t*)&res, _ext2_get_block_offset(dentry->fsdata.sb, cur_block) + offset * 4, 4);
    return res;
}

static uint32_t _ext2_get_block_of_inode_lev1(dentry_t* dentry, uint32_t cur_block, uint32_t inode_block_index)
{
    uint32_t lev_contain = BLOCK_LEN(dentry->fsdata.sb) / 4;
    uint32_t offset = inode_block_index / lev_contain;
    uint32_t offset_inner = inode_block_index % lev_contain;
    uint32_t res;
    _ext2_read_from_dev(dentry->dev, (uint8_t*)&res, _ext2_get_block_offset(dentry->fsdata.sb, cur_block) + offset * 4, 4);
    return res ? _ext2_get_block_of_inode_lev0(dentry, res, offset_inner) : 0;
}

static uint32_t _ext2_get_block_of_inode_lev2(dentry_t* dentry, uint32_t cur_block, uint32_t inode_block_index)
{
    uint32_t block_len = BLOCK_LEN(dentry->fsdata.sb) / 4;
    uint32_t lev_contain = block_len * block_len;
    uint32_t offset = inode_block_index / lev_contain;
    uint32_t offset_inner = inode_block_index % lev_contain;
    uint32_t res;
    _ext2_read_from_dev(dentry->dev, (uint8_t*)&res, _ext2_get_block_offset(dentry->fsdata.sb, cur_block) + offset * 4, 4);
    return res ? _ext2_get_block_of_inode_lev1(dentry, res, offset_inner) : 0;
}

/* FIXME: think of more effecient way */
static uint32_t _ext2_get_block_of_inode(dentry_t* dentry, uint32_t inode_block_index)
{
    uint32_t block_len = BLOCK_LEN(dentry->fsdata.sb) / 4;
    if (inode_block_index < 12) {
        return dentry->inode->block[inode_block_index];
    } else if (inode_block_index < 12 + block_len) { // single indirect
        return _ext2_get_block_of_inode_lev0(dentry, dentry->inode->block[12], inode_block_index - 12);
    } else if (inode_block_index < 12 + block_len + block_len * block_len) { // double indirect
        return _ext2_get_block_of_inode_lev1(dentry, dentry->inode->block[13], inode_block_index - 12 - block_len);
    } else { // triple indirect
        return _ext2_get_block_of_inode_lev2(dentry, dentry->inode->block[14], inode_block_index - (12 + block_len + block_len * block_len));
    }
}

static int _ext2_set_block_of_inode_lev0(dentry_t* dentry, uint32_t cur_block, uint32_t inode_block_index, uint32_t val)
{
    uint32_t offset = inode_block_index;
    _ext2_write_to_dev(dentry->dev, (uint8_t*)&val, _ext2_get_block_offset(dentry->fsdata.sb, cur_block) + offset * 4, 4);
    return 0;
}

static int _ext2_set_block_of_inode_lev1(dentry_t* dentry, uint32_t cur_block, uint32_t inode_block_index, uint32_t val)
{
    uint32_t lev_contain = BLOCK_LEN(dentry->fsdata.sb) / 4;
    uint32_t offset = inode_block_index / lev_contain;
    uint32_t offset_inner = inode_block_index % lev_contain;
    uint32_t res;
    _ext2_read_from_dev(dentry->dev, (uint8_t*)&res, _ext2_get_block_offset(dentry->fsdata.sb, cur_block) + offset * 4, 4);
    return res ? _ext2_set_block_of_inode_lev0(dentry, res, offset_inner, val) : -1;
}

static int _ext2_set_block_of_inode_lev2(dentry_t* dentry, uint32_t cur_block, uint32_t inode_block_index, uint32_t val)
{
    uint32_t block_len = BLOCK_LEN(dentry->fsdata.sb) / 4;
    uint32_t lev_contain = block_len * block_len;
    uint32_t offset = inode_block_index / lev_contain;
    uint32_t offset_inner = inode_block_index % lev_contain;
    uint32_t res;
    _ext2_read_from_dev(dentry->dev, (uint8_t*)&res, _ext2_get_block_offset(dentry->fsdata.sb, cur_block) + offset * 4, 4);
    return res ? _ext2_set_block_of_inode_lev1(dentry, res, offset_inner, val) : -1;
}

/* FIXME: think of more effecient way */
int _ext2_set_block_of_inode(dentry_t* dentry, uint32_t inode_block_index, uint32_t val)
{
    uint32_t block_len = BLOCK_LEN(dentry->fsdata.sb) / 4;
    if (inode_block_index < 12) {
        dentry->inode->block[inode_block_index] = val;
        dentry_set_flag(dentry, DENTRY_DIRTY);
        return 0;
    } else if (inode_block_index < 12 + block_len) { // single indirect
        return _ext2_set_block_of_inode_lev0(dentry, dentry->inode->block[12], inode_block_index - 12, val);
    } else if (inode_block_index < 12 + block_len + block_len * block_len) { // double indirect
        return _ext2_set_block_of_inode_lev1(dentry, dentry->inode->block[13], inode_block_index - 12 - block_len, val);
    } else { // triple indirect
        return _ext2_set_block_of_inode_lev2(dentry, dentry->inode->block[14], inode_block_index - (12 + block_len + block_len * block_len), val);
    }
}

static int _ext2_find_free_block_index(vfs_device_t* dev, fsdata_t fsdata, uint32_t* block_index, uint32_t group_index)
{
    uint8_t block_bitmap[MAX_BLOCK_LEN];
    _ext2_read_from_dev(dev, block_bitmap, _ext2_get_block_offset(fsdata.sb, fsdata.gt->table[group_index].block_bitmap), BLOCK_LEN(fsdata.sb));

    for (uint32_t off = 0; off < BLOCK_LEN(fsdata.sb); off++) {
        if (!_ext2_bitmap_get(block_bitmap, off)) {
            *block_index = fsdata.sb->blocks_per_group * group_index + off + 1;
            _ext2_bitmap_set_bit(block_bitmap, off);
            _ext2_write_to_dev(dev, block_bitmap, _ext2_get_block_offset(fsdata.sb, fsdata.gt->table[group_index].block_bitmap), BLOCK_LEN(fsdata.sb));
            return 0;
        }
    }
    return -ENOSPC;
}

static int _ext2_allocate_block_index(vfs_device_t* dev, fsdata_t fsdata, uint32_t* block_index, uint32_t pref_group)
{
    // TODO: change group count to real value
    uint32_t groups_cnt = 3;
    for (int i = 0; i < groups_cnt; i++) {
        if (GROUP_TABLES[(pref_group + i) % groups_cnt].free_inodes_count) {
            if (_ext2_find_free_block_index(dev, fsdata, block_index, pref_group) == 0) {
                return 0;
            }
        }
    }
    return -ENOSPC;
}

static int _ext2_free_block_index(vfs_device_t* dev, fsdata_t fsdata, uint32_t block_index)
{
    block_index--;
    uint32_t block_len = BLOCK_LEN(fsdata.sb);
    uint32_t group_index = block_index / block_len;
    uint32_t off = block_index % block_len;

    uint8_t block_bitmap[MAX_BLOCK_LEN];
    _ext2_read_from_dev(dev, block_bitmap, _ext2_get_block_offset(fsdata.sb, fsdata.gt->table[group_index].block_bitmap), block_len);

    _ext2_bitmap_unset_bit(block_bitmap, off);
    _ext2_write_to_dev(dev, block_bitmap, _ext2_get_block_offset(fsdata.sb, fsdata.gt->table[group_index].block_bitmap), block_len);
    return 0;
}

/**
 * Returns allocated block in @block_index
 */
static int _ext2_allocate_block_for_inode(dentry_t* dentry, uint32_t pref_group, uint32_t* block_index)
{
    if (_ext2_allocate_block_index(dentry->dev, dentry->fsdata, block_index, pref_group) == 0) {
        uint32_t blocks_per_inode = TO_EXT_BLOCKS_CNT(dentry->fsdata.sb, dentry->inode->blocks);
        if (_ext2_set_block_of_inode(dentry, blocks_per_inode, *block_index) == 0) {
            dentry->inode->blocks += BLOCK_LEN(dentry->fsdata.sb) / 512;
            dentry_set_flag(dentry, DENTRY_DIRTY);
            return 0;
        }
    }
    return -ENOSPC;
}

/**
 * INODE FUNCTIONS
 */

int ext2_read_inode(dentry_t* dentry)
{
    uint32_t inodes_per_group = dentry->fsdata.sb->inodes_per_group;
    uint32_t holder_group = (dentry->inode_indx - 1) / inodes_per_group;
    uint32_t pos_inside_group = (dentry->inode_indx - 1) % inodes_per_group;
    uint32_t inode_start = _ext2_get_block_offset(dentry->fsdata.sb, dentry->fsdata.gt->table[holder_group].inode_table) + (pos_inside_group * INODE_LEN);
    _ext2_read_from_dev(dentry->dev, (uint8_t*)dentry->inode, inode_start, INODE_LEN);
    return 0;
}

int ext2_write_inode(dentry_t* dentry)
{
    uint32_t inodes_per_group = dentry->fsdata.sb->inodes_per_group;
    uint32_t holder_group = (dentry->inode_indx - 1) / inodes_per_group;
    uint32_t pos_inside_group = (dentry->inode_indx - 1) % inodes_per_group;
    uint32_t inode_start = _ext2_get_block_offset(dentry->fsdata.sb, dentry->fsdata.gt->table[holder_group].inode_table) + (pos_inside_group * INODE_LEN);
    _ext2_write_to_dev(dentry->dev, (uint8_t*)dentry->inode, inode_start, INODE_LEN);
    return 0;
}

static int _ext2_find_free_inode_index(vfs_device_t* dev, fsdata_t fsdata, uint32_t* inode_index, uint32_t group_index)
{
    uint8_t inode_bitmap[MAX_BLOCK_LEN];
    _ext2_read_from_dev(dev, inode_bitmap, _ext2_get_block_offset(fsdata.sb, fsdata.gt->table[group_index].inode_bitmap), BLOCK_LEN(fsdata.sb));

    for (uint32_t off = 0; off < BLOCK_LEN(fsdata.sb); off++) {
        if (!_ext2_bitmap_get(inode_bitmap, off)) {
            *inode_index = SUPERBLOCK->inodes_per_group * group_index + off + 1;
            _ext2_bitmap_set_bit(inode_bitmap, off);
            _ext2_write_to_dev(dev, inode_bitmap, _ext2_get_block_offset(fsdata.sb, fsdata.gt->table[group_index].inode_bitmap), BLOCK_LEN(fsdata.sb));
            return 0;
        }
    }
    return -ENOSPC;
}

static int _ext2_allocate_inode_index(vfs_device_t* dev, fsdata_t fsdata, uint32_t* inode_index, uint32_t pref_group)
{
    /* TODO: change group count to real value */
    uint32_t groups_cnt = 3;
    for (int i = 0; i < groups_cnt; i++) {
        if (fsdata.gt->table[(pref_group + i) % groups_cnt].free_inodes_count) {
            if (_ext2_find_free_inode_index(dev, fsdata, inode_index, pref_group) == 0) {
                return 0;
            }
        }
    }
    return -ENOSPC;
}

static int _ext2_free_inode_index(vfs_device_t* dev, fsdata_t fsdata, uint32_t inode_index)
{
    inode_index--;
    uint32_t block_len = BLOCK_LEN(fsdata.sb);
    uint32_t inodes_per_group = fsdata.sb->inodes_per_group;
    uint32_t group_index = inode_index / inodes_per_group;
    uint32_t off = inode_index % inodes_per_group;

    uint8_t inode_bitmap[MAX_BLOCK_LEN];
    _ext2_read_from_dev(dev, inode_bitmap, _ext2_get_block_offset(fsdata.sb, fsdata.gt->table[group_index].inode_bitmap), block_len);

    _ext2_bitmap_unset_bit(inode_bitmap, off);
    _ext2_write_to_dev(dev, inode_bitmap, _ext2_get_block_offset(fsdata.sb, fsdata.gt->table[group_index].inode_bitmap), block_len);
    return 0;
}

static int _ext2_free_inode(dentry_t* dentry)
{
    uint32_t block_per_dir = TO_EXT_BLOCKS_CNT(dentry->fsdata.sb, dentry->inode->blocks);

    /* freeing all data blocks */
    for (int block_index = 0; block_index < block_per_dir; block_index++) {
        uint32_t data_block_index = _ext2_get_block_of_inode(dentry, block_index);
        _ext2_free_block_index(dentry->dev, dentry->fsdata, data_block_index);
    }

    _ext2_free_inode_index(dentry->dev, dentry->fsdata, dentry->inode_indx);
    dentry_force_put(dentry);
    return 0;
}

/**
 * DIR FUNCTIONS
 */

// NOTE: currently only link version is supported.
// TODO: add cache.
static int _ext2_lookup_block(vfs_device_t* dev, fsdata_t fsdata, uint32_t block_index, const char* name, uint32_t len, uint32_t* found_inode_index)
{
    if (block_index == 0) {
        return -EINVAL;
    }

    uint8_t tmp_buf[MAX_BLOCK_LEN];
    _ext2_read_from_dev(dev, tmp_buf, _ext2_get_block_offset(fsdata.sb, block_index), BLOCK_LEN(fsdata.sb));
    dir_entry_t* start_of_entry = (dir_entry_t*)tmp_buf;
    for (;;) {
        if (start_of_entry->inode == 0) {
            return -EFAULT;
        }

        if (start_of_entry->name_len == len) {
            bool is_name_same = true;
            for (int i = 0; i < start_of_entry->name_len; i++) {
                is_name_same &= (name[i] == *((char*)start_of_entry + 8 + i));
            }

            if (is_name_same) {
                *found_inode_index = start_of_entry->inode;
                return 0;
            }
        }

        start_of_entry = (dir_entry_t*)((uint32_t)start_of_entry + start_of_entry->rec_len);
        if ((uint32_t)start_of_entry >= (uint32_t)tmp_buf + BLOCK_LEN(fsdata.sb)) {
            return -EFAULT;
        }
    }
    return -EFAULT;
}

static int _ext2_getdirent_block(vfs_device_t* dev, fsdata_t fsdata, uint32_t block_index, uint32_t* offset, dirent_t* dirent)
{
    if (block_index == 0) {
        return -EINVAL;
    }
    const uint32_t block_len = BLOCK_LEN(fsdata.sb);
    uint32_t internal_offset = *offset % block_len;

    uint8_t tmp_buf[MAX_BLOCK_LEN];
    _ext2_read_from_dev(dev, tmp_buf, _ext2_get_block_offset(fsdata.sb, block_index), block_len);
    for (;;) {
        dir_entry_t* start_of_entry = (dir_entry_t*)((uint32_t)tmp_buf + internal_offset);
        internal_offset += start_of_entry->rec_len;
        *offset += start_of_entry->rec_len;

        if (start_of_entry->inode != 0) {
            int name_len = start_of_entry->name_len;
            if (name_len > 251) {
                kprintf("[VFS] Full name len is unsupported\n");
                name_len = 251;
            }
            memcpy(dirent->name, (char*)start_of_entry + 8, name_len);
            dirent->name[name_len] = '\0';
            return 0;
        }

        if (internal_offset >= block_len) {
            return -EFAULT;
        }
    }
    return -EFAULT;
}

/**
 * _ext2_getdents_block puts dents into @buf from block with index @block_index.
 * @block_index: index block to put from
 * @buf: result buffer
 * @len: len of the buffer @buf
 * @inner_offset: offset of block, shows the start from where to start
 * @scanned_bytes: keeps the result value how many bytes were scanned in this block.
 * return: returns an error (if return < 0) or count of bytes written to @buf.
 */
static int _ext2_getdents_block(vfs_device_t* dev, fsdata_t fsdata, uint32_t block_index, uint8_t* buf, uint32_t len, uint32_t inner_offset, uint32_t* scanned_bytes)
{
    if (block_index == 0) {
        return -EINVAL;
    }
    const uint32_t block_len = BLOCK_LEN(fsdata.sb);
    int already_read = 0;

    uint8_t tmp_buf[MAX_BLOCK_LEN];
    _ext2_read_from_dev(dev, tmp_buf, _ext2_get_block_offset(fsdata.sb, block_index), block_len);
    for (;;) {
        dir_entry_t* start_of_entry = (dir_entry_t*)((uint32_t)tmp_buf + inner_offset);
        uint32_t record_name_len = NORM_FILENAME(start_of_entry->name_len);
        uint32_t real_rec_len = 8 + record_name_len + 1;

        if (real_rec_len > len) {
            /*  There is no space to put this element in the buffer.
                If we havn't written anything, that means we can't fit
                the elem in the buf, so, return an error */
            if (already_read == 0) {
                return -EINVAL;
            }
            return already_read;
        }

        inner_offset += start_of_entry->rec_len;
        *scanned_bytes += start_of_entry->rec_len;
        if (start_of_entry->inode != 0) {
            // Change it here, to have the correct copied data.
            start_of_entry->rec_len = real_rec_len;
            memcpy(buf + already_read, (uint8_t*)start_of_entry, real_rec_len);
            buf[already_read + real_rec_len - 1] = '\0';

            already_read += real_rec_len;
            len -= real_rec_len;
        }

        if (inner_offset >= block_len) {
            return already_read;
        }
    }
    return already_read;
}

static int _ext2_add_first_entry_to_dir_block(vfs_device_t* dev, fsdata_t fsdata, uint32_t block_index, dentry_t* child_dentry, const char* filename, uint32_t len)
{
    if (block_index == 0) {
        return -EINVAL;
    }

    uint32_t record_name_len = NORM_FILENAME(len);
    uint32_t min_rec_len = 8 + record_name_len;
    dir_entry_t new_entry;

    uint8_t tmp_buf[DIR_ENTRY_LEN];
    _ext2_read_from_dev(dev, tmp_buf, _ext2_get_block_offset(fsdata.sb, block_index), DIR_ENTRY_LEN);
    dir_entry_t* start_of_entry = (dir_entry_t*)tmp_buf;
    new_entry.inode = child_dentry->inode_indx;
    new_entry.rec_len = BLOCK_LEN(fsdata.sb);
    new_entry.name_len = len;
    memcpy((void*)start_of_entry, (void*)&new_entry, 8);
    memcpy((void*)((uint32_t)start_of_entry + 8), (void*)filename, len);
    memset((void*)((uint32_t)start_of_entry + 8 + len), 0, record_name_len - len);
    _ext2_write_to_dev(dev, tmp_buf, _ext2_get_block_offset(fsdata.sb, block_index), DIR_ENTRY_LEN);
    return 0;
}

static int _ext2_add_to_dir_block(vfs_device_t* dev, fsdata_t fsdata, uint32_t block_index, dentry_t* child_dentry, const char* filename, uint32_t len)
{
    if (block_index == 0) {
        return -EINVAL;
    }

    uint32_t record_name_len = NORM_FILENAME(len);
    uint32_t min_rec_len = 8 + record_name_len;
    dir_entry_t new_entry;

    uint8_t tmp_buf[MAX_BLOCK_LEN];
    _ext2_read_from_dev(dev, tmp_buf, _ext2_get_block_offset(fsdata.sb, block_index), BLOCK_LEN(fsdata.sb));
    dir_entry_t* start_of_entry = (dir_entry_t*)tmp_buf;
    dir_entry_t* start_of_new_entry;

    if (start_of_entry->inode == 0) {
        kpanic("Ext2: can't add as first entry with help of that function.");
    }

    for (;;) {
        uint32_t cur_filename_len = NORM_FILENAME(start_of_entry->name_len);
        uint32_t cur_rec_len = 8 + cur_filename_len;

        // We have enough place to put both records
        if (start_of_entry->rec_len >= cur_rec_len + min_rec_len) {
            new_entry.inode = child_dentry->inode_indx;
            new_entry.rec_len = start_of_entry->rec_len - cur_rec_len;
            new_entry.name_len = len;
            start_of_new_entry = (dir_entry_t*)((uint32_t)start_of_entry + cur_rec_len);
            start_of_entry->rec_len = cur_rec_len;
            goto update_res;
        }

        start_of_entry = (dir_entry_t*)((uint32_t)start_of_entry + start_of_entry->rec_len);
        if ((uint32_t)start_of_entry >= (uint32_t)tmp_buf + BLOCK_LEN(fsdata.sb)) {
            return -EFAULT;
        }
    }

update_res:
    memcpy((void*)start_of_new_entry, (void*)&new_entry, 8);
    memcpy((void*)((uint32_t)start_of_new_entry + 8), (void*)filename, len);
    memset((void*)((uint32_t)start_of_new_entry + 8 + len), 0, record_name_len - len);
    _ext2_write_to_dev(dev, tmp_buf, _ext2_get_block_offset(fsdata.sb, block_index), BLOCK_LEN(fsdata.sb));
    return 0;
}

static int _ext2_rm_from_dir_block(vfs_device_t* dev, fsdata_t fsdata, uint32_t block_index, dentry_t* child_dentry)
{
    if (block_index == 0) {
        return -EINVAL;
    }

    uint8_t tmp_buf[MAX_BLOCK_LEN];
    _ext2_read_from_dev(dev, tmp_buf, _ext2_get_block_offset(fsdata.sb, block_index), BLOCK_LEN(fsdata.sb));
    dir_entry_t* start_of_entry = (dir_entry_t*)tmp_buf;
    dir_entry_t* prev_entry = (dir_entry_t*)0;

    for (;;) {

        if (start_of_entry->inode == child_dentry->inode_indx) {
            /* FIXME: just fix that */
            if (!prev_entry) {
                kpanic("Ext2: can't delete first entry.");
            }

            /* deleting entry */
            start_of_entry->inode = 0;
            prev_entry->rec_len += start_of_entry->rec_len;

            _ext2_write_to_dev(dev, tmp_buf, _ext2_get_block_offset(fsdata.sb, block_index), BLOCK_LEN(fsdata.sb));

            return 0;
        }

        prev_entry = start_of_entry;
        start_of_entry = (dir_entry_t*)((uint32_t)start_of_entry + start_of_entry->rec_len);
        if ((uint32_t)start_of_entry >= (uint32_t)tmp_buf + BLOCK_LEN(fsdata.sb)) {
            return -EFAULT;
        }
    }
}

static int _ext2_add_child(dentry_t* dir, dentry_t* child_dentry, const char* name, int len)
{
    uint32_t block_index;
    uint32_t blocks_per_dir = TO_EXT_BLOCKS_CNT(dir->fsdata.sb, dir->inode->blocks);

    for (int i = 0; i < blocks_per_dir; i++) {
        if (block_index = _ext2_get_block_of_inode(dir, i)) {
            if (_ext2_add_to_dir_block(dir->dev, dir->fsdata, block_index, child_dentry, name, len) == 0) {
                goto updated_inode;
            }
        }
    }

    // FIXME: group
    uint32_t new_block_index;
    if (_ext2_allocate_block_for_inode(dir, 0, &new_block_index) == 0) {
        if (_ext2_add_first_entry_to_dir_block(dir->dev, dir->fsdata, new_block_index, child_dentry, name, len) == 0) {
            goto updated_inode;
        }
    }

    return -EFAULT;

updated_inode:
    child_dentry->inode->links_count++;
    dentry_set_flag(child_dentry, DENTRY_DIRTY);
    return 0;
}

static int _ext2_rm_child(dentry_t* dir, dentry_t* child_dentry)
{
    uint32_t block_index;
    uint32_t blocks_per_dir = TO_EXT_BLOCKS_CNT(dir->fsdata.sb, dir->inode->blocks);

    for (int i = 0; i < blocks_per_dir; i++) {
        if (block_index = _ext2_get_block_of_inode(dir, i)) {
            if (_ext2_rm_from_dir_block(dir->dev, dir->fsdata, block_index, child_dentry) == 0) {
                child_dentry->inode->links_count--;
                dentry_set_flag(child_dentry, DENTRY_DIRTY);
                return 0;
            }
        }
    }

    return -ENOENT;
}

static int _ext2_setup_dir(dentry_t* dir, dentry_t* parent_dir, mode_t mode)
{
    dir->inode->mode = mode;
    dir->inode->uid = 0; // FIXME: uid of real user
    dir->inode->links_count = 0;
    dir->inode->blocks = 0;
    dentry_set_flag(dir, DENTRY_DIRTY);
    if (_ext2_add_child(dir, dir, ".", 1) < 0) {
        return -EFAULT;
    }
    if (_ext2_add_child(dir, parent_dir, "..", 2) < 0) {
        return -EFAULT;
    }
    return 0;
}

/**
 * FILE FUNCTIONS
 */

static int _ext2_setup_file(dentry_t* file, mode_t mode)
{
    file->inode->mode = mode;
    file->inode->uid = 0; // FIXME: uid of real user
    file->inode->links_count = 0;
    file->inode->blocks = 0;
    file->inode->size = 0;
    dentry_set_flag(file, DENTRY_DIRTY);
    return 0;
}

/**
 * API FUNTIONS
 */

bool ext2_can_read(dentry_t* dentry)
{
    return true;
}

bool ext2_can_write(dentry_t* dentry)
{
    return true;
}

int ext2_read(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len)
{
    const uint32_t block_len = BLOCK_LEN(dentry->fsdata.sb);
    uint32_t start_block_index = start / block_len;
    uint32_t end_block_index = (start + len) / block_len;
    uint32_t read_offset = start % block_len;
    uint32_t already_read = 0;

    for (uint32_t block_index = start_block_index; block_index <= end_block_index; block_index++) {
        uint32_t data_block_index = _ext2_get_block_of_inode(dentry, block_index);
        uint32_t read_from_block = MIN(len, block_len - read_offset);
        _ext2_read_from_dev(dentry->dev, buf + already_read, _ext2_get_block_offset(dentry->fsdata.sb, data_block_index) + read_offset, read_from_block);
        len -= read_from_block;
        already_read += read_from_block;
        read_offset = 0;
    }

    return already_read;
}

int ext2_write(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len)
{
    const uint32_t block_len = BLOCK_LEN(dentry->fsdata.sb);
    uint32_t start_block_index = start / block_len;
    uint32_t end_block_index = (start + len) / block_len;
    uint32_t write_offset = start % block_len;
    uint32_t to_write = len;
    uint32_t already_written = start % block_len;
    uint32_t blocks_allocated = TO_EXT_BLOCKS_CNT(dentry->fsdata.sb, dentry->inode->blocks);

    for (uint32_t data_block_index, block_index = start_block_index; block_index <= end_block_index; block_index++) {
        uint32_t write_to_block = MIN(to_write, block_len - write_offset);

        if (blocks_allocated <= block_index) {
            _ext2_allocate_block_for_inode(dentry, 0, &data_block_index);
        } else {
            data_block_index = _ext2_get_block_of_inode(dentry, block_index);
        }

        _ext2_write_to_dev(dentry->dev, buf + already_written, _ext2_get_block_offset(dentry->fsdata.sb, data_block_index) + write_offset, write_to_block);
        to_write -= write_to_block;
        already_written += write_to_block;
        write_offset = 0;
    }

    if (dentry->inode->size != start + len) {
        dentry->inode->size = start + len;
    }
    dentry->inode->mtime = (uint32_t)timeman_now();
    dentry_set_flag(dentry, DENTRY_DIRTY);

    return already_written;
}

int ext2_lookup(dentry_t* dir, const char* name, uint32_t len, uint32_t* res_inode_indx)
{
    uint32_t block_per_dir = TO_EXT_BLOCKS_CNT(dir->fsdata.sb, dir->inode->blocks);
    for (int block_index = 0; block_index < block_per_dir; block_index++) {
        uint32_t data_block_index = _ext2_get_block_of_inode(dir, block_index);
        if (_ext2_lookup_block(dir->dev, dir->fsdata, data_block_index, name, len, res_inode_indx) == 0) {
            return 0;
        }
    }
    return -ENOENT;
}

int ext2_mkdir(dentry_t* dir, const char* name, uint32_t len, mode_t mode)
{
    uint32_t new_dir_inode_indx = 0;
    if (_ext2_allocate_inode_index(dir->dev, dir->fsdata, &new_dir_inode_indx, 0) < 0) {
        return -ENOSPC;
    }

    dentry_t* new_dir = dentry_get(dir->dev_indx, new_dir_inode_indx);

    if (_ext2_setup_dir(new_dir, dir, mode) < 0) {
        dentry_put(new_dir);
        return -EFAULT;
    }
    if (_ext2_add_child(dir, new_dir, name, len) < 0) {
        dentry_put(new_dir);
        return -EFAULT;
    }

    dentry_put(new_dir);
    return 0;
}

int ext2_getdirent(dentry_t* dir, uint32_t* offset, dirent_t* res)
{
    const uint32_t block_len = BLOCK_LEN(dir->fsdata.sb);
    uint32_t blocks_per_dir = TO_EXT_BLOCKS_CNT(dir->fsdata.sb, dir->inode->blocks);
    if (*offset >= blocks_per_dir * block_len) {
        return -1;
    }

    for (uint32_t block_index = *offset / block_len; block_index < blocks_per_dir; block_index++) {
        uint32_t data_block_index = _ext2_get_block_of_inode(dir, block_index);
        if (_ext2_getdirent_block(dir->dev, dir->fsdata, data_block_index, offset, res) == 0) {
            return 0;
        }
    }

    return 0;
}

int ext2_getdents(dentry_t* dentry, uint8_t* buf, uint32_t* offset, uint32_t len)
{
    const uint32_t block_len = BLOCK_LEN(dentry->fsdata.sb);
    uint32_t start_block_index = *offset / block_len;
    uint32_t end_block_index = TO_EXT_BLOCKS_CNT(dentry->fsdata.sb, dentry->inode->blocks);
    uint32_t read_offset = *offset % block_len;
    uint32_t already_read = 0;

    for (uint32_t block_index = start_block_index; block_index < end_block_index; block_index++) {
        uint32_t data_block_index = _ext2_get_block_of_inode(dentry, block_index);
        uint32_t read_from_block = MIN(len, block_len - read_offset);
        int act_read = _ext2_getdents_block(dentry->dev, dentry->fsdata, data_block_index, buf + already_read, read_from_block, read_offset, offset);
        if (act_read < 0) {
            if (already_read == 0) {
                return act_read;
            }
            return already_read;
        }
        len -= act_read;
        already_read += act_read;
        read_offset = 0;
    }

    return already_read;
}

int ext2_create(dentry_t* dir, const char* name, uint32_t len, mode_t mode)
{
    uint32_t new_file_inode_indx = 0;
    if (_ext2_allocate_inode_index(dir->dev, dir->fsdata, &new_file_inode_indx, 0) < 0) {
        return -ENOSPC;
    }
    dentry_t* new_file = dentry_get(dir->dev_indx, new_file_inode_indx);

    if (_ext2_setup_file(new_file, mode) < 0) {
        dentry_put(new_file);
        return -EFAULT;
    }

    if (_ext2_add_child(dir, new_file, name, len) < 0) {
        dentry_put(new_file);
        return -EFAULT;
    }

    dentry_put(new_file);
    return 0;
}

int ext2_rm(dentry_t* dentry)
{
    dentry_t* parent_dir = dentry_get_parent(dentry);

    if (!parent_dir) {
        // Can't delete root
        return -EPERM;
    }

    if (_ext2_rm_child(parent_dir, dentry) < 0) {
        return -EFAULT;
    }

    if (_ext2_free_inode(dentry) < 0) {
        return -EFAULT;
    }

    return 0;
}

/* TODO return int */
bool ext2_recognize_drive(vfs_device_t* dev)
{
    superblock_t* superblock = (superblock_t*)kmalloc(SUPERBLOCK_LEN);
    _ext2_read_from_dev(dev, (uint8_t*)superblock, SUPERBLOCK_START, SUPERBLOCK_LEN);

    if (superblock->magic != 0xEF53) {
        kfree(superblock);
        return false;
    }
    if (superblock->rev_level != 0) {
        kfree(superblock);
        return false;
    }

    kfree(superblock);

    return true;
}

int ext2_prepare_fs(vfs_device_t* dev)
{
    superblock_t* superblock = (superblock_t*)kmalloc(SUPERBLOCK_LEN);
    _ext2_read_from_dev(dev, (uint8_t*)superblock, SUPERBLOCK_START, SUPERBLOCK_LEN);
    _ext2_superblocks[dev->dev->id] = superblock;

    // FIXME: for now we consider that we have at max 5 groups
    uint32_t groups_cnt = _ext2_get_groups_cnt(dev, superblock);
    uint32_t group_table_len = groups_cnt * GROUP_LEN;
    group_desc_t* group_table = (group_desc_t*)kmalloc(group_table_len);
    _ext2_read_from_dev(dev, (uint8_t*)group_table, _ext2_get_block_offset(superblock, 2), group_table_len);

    _ext2_group_table_info[dev->dev->id].count = groups_cnt;
    _ext2_group_table_info[dev->dev->id].table = group_table;

    return 0;
}

void ext2_save_state(vfs_device_t* dev)
{
    if (!_ext2_superblocks[dev->dev->id]) {
        return;
    }

    superblock_t* superblock = _ext2_superblocks[dev->dev->id];

    // FIXME: for now we consider that we have at max 5 groups
    uint32_t group_table_len = _ext2_group_table_info[dev->dev->id].count * GROUP_LEN;
    group_desc_t* group_table = _ext2_group_table_info[dev->dev->id].table;
    _ext2_write_to_dev(dev, (uint8_t*)group_table, _ext2_get_block_offset(superblock, 2), group_table_len);
    kfree(group_table);

    _ext2_write_to_dev(dev, (uint8_t*)superblock, SUPERBLOCK_START, SUPERBLOCK_LEN);
    kfree(superblock);
}

fsdata_t get_fsdata(dentry_t* dentry)
{
    fsdata_t fsdata;
    fsdata.sb = _ext2_superblocks[dentry->dev_indx];
    fsdata.gt = &_ext2_group_table_info[dentry->dev_indx];
    return fsdata;
}

/**
 * INIT FUNCTIONS
 */

driver_desc_t _ext2_driver_info()
{
    driver_desc_t fs_desc;
    fs_desc.type = DRIVER_FILE_SYSTEM;
    fs_desc.auto_start = false;
    fs_desc.is_device_driver = false;
    fs_desc.is_device_needed = false;
    fs_desc.is_driver_needed = false;
    fs_desc.functions[DRIVER_FILE_SYSTEM_RECOGNIZE] = ext2_recognize_drive;
    fs_desc.functions[DRIVER_FILE_SYSTEM_PREPARE_FS] = ext2_prepare_fs;
    fs_desc.functions[DRIVER_FILE_SYSTEM_CAN_READ] = ext2_can_read;
    fs_desc.functions[DRIVER_FILE_SYSTEM_CAN_WRITE] = ext2_can_write;
    fs_desc.functions[DRIVER_FILE_SYSTEM_READ] = ext2_read;
    fs_desc.functions[DRIVER_FILE_SYSTEM_WRITE] = ext2_write;
    fs_desc.functions[DRIVER_FILE_SYSTEM_MKDIR] = ext2_mkdir;
    fs_desc.functions[DRIVER_FILE_SYSTEM_EJECT_DEVICE] = ext2_save_state;

    fs_desc.functions[DRIVER_FILE_SYSTEM_READ_INODE] = ext2_read_inode;
    fs_desc.functions[DRIVER_FILE_SYSTEM_WRITE_INODE] = ext2_write_inode;
    fs_desc.functions[DRIVER_FILE_SYSTEM_GET_FSDATA] = get_fsdata;
    fs_desc.functions[DRIVER_FILE_SYSTEM_LOOKUP] = ext2_lookup;
    fs_desc.functions[DRIVER_FILE_SYSTEM_GETDENTS] = ext2_getdents;
    fs_desc.functions[DRIVER_FILE_SYSTEM_CREATE] = ext2_create;
    fs_desc.functions[DRIVER_FILE_SYSTEM_RM] = ext2_rm;

    fs_desc.functions[DRIVER_FILE_SYSTEM_IOCTL] = 0;

    return fs_desc;
}

void ext2_install()
{
    driver_install(_ext2_driver_info());
}