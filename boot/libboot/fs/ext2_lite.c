/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libboot/fs/ext2_lite.h>
#include <libboot/mem/mem.h>
#include <libboot/types.h>

superblock_t superblock;
drive_desc_t* active_drive_desc;

uint32_t _ext2_lite_get_block_len()
{
    return (1 << (superblock.log_block_size + 10));
}

uint32_t _ext2_lite_get_offset_of_block(uint32_t block_index)
{
    return SUPERBLOCK_START + (block_index - 1) * _ext2_lite_get_block_len();
}

static uint8_t tmp_read_buf[8192];
void _ext2_lite_read(uint8_t* buf, uint32_t start, uint32_t len)
{
    void (*read)(uint32_t sector, uint8_t * read_to) = active_drive_desc->read;
    int already_read = 0;
    while (len != 0) {
        uint32_t sector = start / 512;
        uint32_t start_offset = start % 512;
        read(sector, tmp_read_buf);
        for (int i = 0; i < min(512 - start_offset, len); i++) {
            buf[already_read++] = tmp_read_buf[start_offset + i];
        }
        len -= min(512 - start_offset, len);
        start += min(512 - start_offset, len);
    }
}

void _ext2_lite_read_group(uint32_t group_id, group_desc_t* group)
{
    uint32_t group_start = SUPERBLOCK_START + _ext2_lite_get_block_len() + (group_id * GROUP_LEN);
    _ext2_lite_read((uint8_t*)group, group_start, GROUP_LEN);
}

void _ext2_lite_read_inode(uint32_t inode_id, inode_t* inode)
{
    uint32_t inodes_per_group = superblock.inodes_per_group;
    uint32_t holder_group = (inode_id - 1) / inodes_per_group;
    uint32_t pos_inside_group = (inode_id - 1) % inodes_per_group;

    group_desc_t gr;
    _ext2_lite_read_group(holder_group, &gr);

    uint32_t inode_start = SUPERBLOCK_START + ((gr.inode_table - 1) * _ext2_lite_get_block_len()) + (pos_inside_group * INODE_LEN);
    _ext2_lite_read((uint8_t*)inode, inode_start, INODE_LEN);
}

int ext2_lite_init(drive_desc_t* drive_desc, fs_desc_t* fs_desc)
{
    active_drive_desc = drive_desc;
    void (*read)(uint32_t sector, uint8_t * read_to) = drive_desc->read;
    _ext2_lite_read((uint8_t*)&superblock, SUPERBLOCK_START, SUPERBLOCK_LEN);

    if (superblock.magic != 0xEF53) {
        return -1;
    }
    if (superblock.rev_level != 0) {
        return -1;
    }

    // TODO currently support up to 8192 bytes a block.
    if (superblock.log_block_size > 3) {
        return -1;
    }

    fs_desc->get_inode = (void*)ext2_lite_get_inode;
    fs_desc->read = ext2_lite_read;
    fs_desc->read_from_inode = (void*)ext2_lite_read_inode;
    return 0;
}

static uint8_t tmp_dir_buf[8192];
int ext2_lite_has_in_dir(uint32_t block_index, const char* path, uint32_t* found_inode_index)
{
    if (block_index == 0) {
        return -1;
    }
    if (_ext2_lite_get_block_len() != 1024) {
        while (1) { }
        return -1;
    }

    _ext2_lite_read(tmp_dir_buf, _ext2_lite_get_offset_of_block(block_index), _ext2_lite_get_block_len());
    dir_entry_t* start_of_entry = (dir_entry_t*)tmp_dir_buf;
    for (;;) {
        if (start_of_entry->inode == 0) {
            return -1;
        }
        // checking name of this entry
        bool is_name_correct = true;
        for (int i = 0; i < start_of_entry->name_len; i++) {
            is_name_correct &= (path[i] == *((char*)start_of_entry + 8 + i));
        }
        is_name_correct &= (path[start_of_entry->name_len] == '\0' || path[start_of_entry->name_len] == '/');

        if (is_name_correct) {
            *found_inode_index = start_of_entry->inode;
            return 0;
        }

        start_of_entry = (dir_entry_t*)((uint32_t)start_of_entry + start_of_entry->rec_len);
        if ((uint32_t)start_of_entry >= (uint32_t)tmp_dir_buf + _ext2_lite_get_block_len()) {
            return -1;
        }
    }
    return -2;
}

int ext2_lite_scan_dir(inode_t inode, const char* path, inode_t* res_inode)
{
    uint32_t nxt_inode_index;
    uint32_t path_offset = 0;
new_inode:
    for (int i = 0; i < 12; i++) {
        if (inode.block[i] != 0) {
            if (ext2_lite_has_in_dir(inode.block[i], &path[path_offset], &nxt_inode_index) == 0) {
                while (path[path_offset] != '\0' && path[path_offset] != '/')
                    path_offset++;
                for (; path[path_offset] != '/'; path_offset++) {
                    if (path[path_offset] == '\0') {
                        _ext2_lite_read_inode(nxt_inode_index, res_inode);
                        return 0;
                    }
                }
                path_offset++;
                _ext2_lite_read_inode(nxt_inode_index, &inode);
                goto new_inode;
            }
        }
    }
    return -1;
}

int ext2_lite_get_inode(drive_desc_t* drive_desc, const char* path, inode_t* file_inode)
{
    active_drive_desc = drive_desc;
    inode_t root_inode;
    _ext2_lite_read_inode(2, &root_inode);

    int res = -1;
    if (path[0] == '/') {
        res = ext2_lite_scan_dir(root_inode, &path[1], file_inode);
    } else {
        res = ext2_lite_scan_dir(root_inode, path, file_inode);
    }

    return res;
}

static uint8_t tmp_block_buf[8192];
uint32_t _ext2_lite_get_block_of_file_lev0(uint32_t cur_block, uint32_t file_block_index)
{
    _ext2_lite_read(tmp_block_buf, _ext2_lite_get_offset_of_block(cur_block), _ext2_lite_get_block_len());
    uint32_t* buf = (uint32_t*)tmp_block_buf;
    return buf[file_block_index];
}

uint32_t _ext2_lite_get_block_of_file_lev1(uint32_t cur_block, uint32_t file_block_index)
{
    _ext2_lite_read(tmp_block_buf, _ext2_lite_get_offset_of_block(cur_block), _ext2_lite_get_block_len());
    uint32_t* buf = (uint32_t*)tmp_block_buf;

    uint32_t lev_contain = _ext2_lite_get_block_len() / 4;
    uint32_t offset = file_block_index / lev_contain;
    uint32_t offset_inner = file_block_index % lev_contain;

    return _ext2_lite_get_block_of_file_lev0(buf[offset], offset_inner);
}

uint32_t _ext2_lite_get_block_of_file_lev2(uint32_t cur_block, uint32_t file_block_index)
{
    _ext2_lite_read(tmp_block_buf, _ext2_lite_get_offset_of_block(cur_block), _ext2_lite_get_block_len());
    uint32_t* buf = (uint32_t*)tmp_block_buf;

    uint32_t block_len = _ext2_lite_get_block_len() / 4;
    uint32_t lev_contain = block_len * block_len;
    uint32_t offset = file_block_index / lev_contain;
    uint32_t offset_inner = file_block_index % lev_contain;

    return _ext2_lite_get_block_of_file_lev1(buf[offset], offset_inner);
}

// TODO think of more effecient version
uint32_t _ext2_lite_get_block_of_file(inode_t* inode, uint32_t file_block_index)
{
    uint32_t block_len = _ext2_lite_get_block_len() / 4;
    if (file_block_index < 12) {
        return inode->block[file_block_index];
    } else if (file_block_index < 12 + block_len) { // single indirect
        return _ext2_lite_get_block_of_file_lev0(inode->block[12], file_block_index - 12);
    } else if (file_block_index < 12 + block_len + block_len * block_len) { // double indirect
        return _ext2_lite_get_block_of_file_lev1(inode->block[13], file_block_index - 12 - block_len);
    } else { // triple indirect
        return _ext2_lite_get_block_of_file_lev2(inode->block[14], file_block_index - (12 + block_len + block_len * block_len));
    }
}

// Note to save mem: the func reuses tmp_block_buf.
int ext2_lite_read_inode(drive_desc_t* drive_desc, inode_t* inode, uint8_t* buf, uint32_t from, uint32_t len)
{
    active_drive_desc = drive_desc;

    const uint32_t block_len = _ext2_lite_get_block_len();
    uint32_t start_block_index = from / block_len;
    uint32_t end_block_index = (from + len - 1) / block_len;
    uint32_t read_offset = from % block_len;
    uint32_t write_offset = 0;

    for (uint32_t block_index = start_block_index; block_index <= end_block_index; block_index++) {
        uint32_t data_block_index = _ext2_lite_get_block_of_file(inode, block_index);
        _ext2_lite_read(tmp_block_buf, _ext2_lite_get_offset_of_block(data_block_index), _ext2_lite_get_block_len());
        for (int i = 0; i < min(len, block_len - read_offset); i++) {
            buf[write_offset++] = tmp_block_buf[read_offset + i];
        }
        len -= min(len, block_len - read_offset);
        read_offset = 0;
    }
    return write_offset;
}

int ext2_lite_read(drive_desc_t* drive_desc, const char* path, uint8_t* buf, uint32_t from, uint32_t len)
{
    active_drive_desc = drive_desc;
    inode_t inode;
    ext2_lite_get_inode(drive_desc, path, &inode);
    return ext2_lite_read_inode(drive_desc, &inode, buf, from, len);
}