/**
 * Ext2 Lite is lightweight driver only for reading Ext2 filesystem
 */

#include "ext2_lite.h"
#include "display.h"

superblock_t superblock;
drive_desc_t *active_drive_desc;

uint32_t _ext2_lite_min(uint32_t a, uint32_t b) {
    if (a < b) {
        return a;
    }
    return b;
}

uint32_t _ext2_lite_get_block_len() {
    return (1 << (superblock.log_block_size + 10));
}

uint32_t _ext2_lite_get_offset_of_block(uint32_t block_index) {
    return SUPERBLOCK_START + (block_index - 1) * _ext2_lite_get_block_len();
}

void _ext2_lite_read(uint8_t *buf, uint32_t start, uint32_t len) {
    uint8_t tmp_buf[512];
    void (*read)(uint32_t sector, uint8_t* read_to) = active_drive_desc->read;
    int already_read = 0;
    while (len != 0) {
        uint32_t sector = start / 512;
        uint32_t start_offset = start % 512;
        read(sector, tmp_buf);
        for (int i = 0; i < _ext2_lite_min(512 - start_offset, len); i++) {
            buf[already_read++] = tmp_buf[start_offset + i];
        }
        len -= _ext2_lite_min(512 - start_offset, len);
        start += _ext2_lite_min(512 - start_offset, len); 
    }
}

void _ext2_lite_read_group(uint32_t group_id, group_desc_t *group) {
    uint32_t group_start = SUPERBLOCK_START + _ext2_lite_get_block_len() + (group_id * GROUP_LEN);
    _ext2_lite_read((uint8_t*)group, group_start, GROUP_LEN);
}

void _ext2_lite_read_inode(uint32_t inode_id, inode_t *inode) {
    uint32_t inodes_per_group = superblock.inodes_per_group;
    uint32_t holder_group = (inode_id - 1) / inodes_per_group;
    uint32_t pos_inside_group = (inode_id - 1) % inodes_per_group;
    
    group_desc_t gr;
    _ext2_lite_read_group(holder_group, &gr);
    
    uint32_t inode_start = SUPERBLOCK_START + ((gr.inode_table - 1) * _ext2_lite_get_block_len()) + (pos_inside_group * INODE_LEN);
    _ext2_lite_read((uint8_t*)inode, inode_start, INODE_LEN);
}

int ext2_lite_init(drive_desc_t *drive_desc, fs_desc_t *fs_desc) {
    active_drive_desc = drive_desc;
    void (*read)(uint32_t sector, uint8_t* read_to) = drive_desc->read;
    _ext2_lite_read((uint8_t*)&superblock, SUPERBLOCK_START, SUPERBLOCK_LEN);

    printd(superblock.log_block_size); printf("\n");

    if (superblock.magic != 0xEF53) {
        return -1;
    }
    if (superblock.rev_level != 0) {
        return -1;
    }
    fs_desc->read = ext2_lite_read;
    return 0;
}

int ext2_lite_has_in_dir(uint32_t block_index, char *path, uint32_t *found_inode_index) {
    if (block_index == 0) {
        return -1;
    }
    if (_ext2_lite_get_block_len() != 1024) {
        printf("oneLO: Block size is unsupported");
        while(1) {}
        return -1;
    }
    // TODO assume a 1024 block for now only;
    uint8_t block_data[1024];
    printd(block_index);
    _ext2_lite_read(block_data, _ext2_lite_get_offset_of_block(block_index), _ext2_lite_get_block_len());
    dir_entry_t* start_of_entry = (dir_entry_t*)block_data;
    // TODO mightn't stop at right time
    for (;;) {
        if (start_of_entry->inode == 0) {
            return -1;
        }
        // checking name of this entry
        bool is_name_correct = true;
        printf((char*)start_of_entry+8); printf("\n");
        // while (1) {}

        for (int i = 0; i < start_of_entry->name_len; i++) {
            is_name_correct &= (path[i] == *((char*)start_of_entry+8+i));
            
        }
        is_name_correct &= (path[start_of_entry->name_len] == '\0' || path[start_of_entry->name_len] == '/');
        
        // if (start_of_entry->name_len == 4) {
        //     printf(path);
        //     while (1) {}
        // }

        if (is_name_correct) {
            *found_inode_index = start_of_entry->inode;
            return 0;
        }
        
        start_of_entry = (dir_entry_t*)((uint32_t)start_of_entry + start_of_entry->rec_len);
        if ((uint32_t)start_of_entry >= (uint32_t)block_data + _ext2_lite_get_block_len()) {
            return -1;
        }
    }
    return -2;
}

int ext2_lite_scan_dir(inode_t inode, char *path, inode_t *res_inode) {
    uint32_t nxt_inode_index;
    uint32_t path_offset = 0;
    for (;;) {
new_inode:
        for (int i = 0; i < 12; i++) {
            if (inode.block[i] != 0) {
                if (ext2_lite_has_in_dir(inode.block[i], &path[path_offset], &nxt_inode_index) == 0) {
                    // printf("Found\n");
                    while (path[path_offset] != '\0' && path[path_offset] != '/') path_offset++;
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
    }
    return -1;
}

int ext2_lite_read(drive_desc_t *drive_desc, char *path, uint8_t *read_to) {
    active_drive_desc = drive_desc;
    inode_t root_inode;
    inode_t file_inode;
    _ext2_lite_read_inode(2, &root_inode);

    int res = -1;
    if (path[0] == '/') {
        res = ext2_lite_scan_dir(root_inode, &path[1], &file_inode);
    } else {
        res = ext2_lite_scan_dir(root_inode, path, &file_inode);
    }

    printd(file_inode.size);

    if (res == 0) {
        printf("OK_file\n");
    }

    // _ext2_lite_read_inode(2, &root_inode);

    // printd(root_inode.mode);

    if (_ext2_lite_get_block_len() == 1024) {
        printf("OK\n");
    }

    while (1) {}
}