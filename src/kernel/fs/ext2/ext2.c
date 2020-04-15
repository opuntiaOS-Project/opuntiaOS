#include <fs/ext2/ext2.h>
#include <mem/malloc.h>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define ext2_get_superblock() _ext2_superblocks[dev->dev.id]
#define ext2_get_group_tables() _ext2_group_tables[dev->dev.id]

static superblock_t *_ext2_superblocks[MAX_DEVICES_COUNT];
static group_desc_t *_ext2_group_tables[MAX_DEVICES_COUNT];

driver_desc_t _ext2_driver_info();

/* TOOLS */
void _ext2_read_from_dev(vfs_device_t *dev, uint8_t *buf, uint32_t start, uint32_t len);
void _ext2_write_to_dev(vfs_device_t *dev, uint8_t *buf, uint32_t start, uint32_t len);
uint32_t _ext2_get_block_len(vfs_device_t *dev);
uint32_t _ext2_get_groups_cnt(vfs_device_t *dev);
uint32_t _ext2_get_block_offset(vfs_device_t *dev, uint32_t block_idx);


void _ext2_read_from_dev(vfs_device_t *dev, uint8_t *buf, uint32_t start, uint32_t len) {
    void (*read)(device_t *d, uint32_t s, uint8_t *r) =
        drivers[dev->dev.driver_id].driver_desc.functions[DRIVER_STORAGE_READ];
    int already_read = 0;
    uint8_t tmp_buf[512];
    while (len != 0) {
        uint32_t sector = start / 512;
        uint32_t start_offset = start % 512;
        read(&dev->dev, sector, tmp_buf);
        for (int i = 0; i < MIN(512 - start_offset, len); i++) {
            buf[already_read++] = tmp_buf[start_offset + i];
        }
        len -= MIN(512 - start_offset, len);
        start += MIN(512 - start_offset, len); 
    }
}

void _ext2_write_to_dev(vfs_device_t *dev, uint8_t *buf, uint32_t start, uint32_t len) {
    void (*read)(device_t *d, uint32_t s, uint8_t *r) =
        drivers[dev->dev.driver_id].driver_desc.functions[DRIVER_STORAGE_READ];
    void (*write)(device_t *d, uint32_t s, uint8_t *r, uint32_t siz) =
        drivers[dev->dev.driver_id].driver_desc.functions[DRIVER_STORAGE_WRITE];
    int already_written = 0;
    uint8_t tmp_buf[512];
    while (len != 0) {
        uint32_t sector = start / 512;
        uint32_t start_offset = start % 512;
        if (start_offset != 0 || len < 512) {
            printf("READ SECTOR\n");
            read(&dev->dev, sector, tmp_buf);
        }
        printf("OFFSET"); printd(start_offset); printf("\n");
        for (int i = 0; i < MIN(512 - start_offset, len); i++) {
            tmp_buf[start_offset + i] = buf[already_written++];
        }
        write(&dev->dev, sector, tmp_buf, 512);
        len -= MIN(512 - start_offset, len);
        start += MIN(512 - start_offset, len);
    }
}


uint32_t _ext2_get_block_len(vfs_device_t *dev) {
    return (1 << (ext2_get_superblock()->log_block_size + 10));
}

uint32_t _ext2_get_block_offset(vfs_device_t *dev, uint32_t block_idx) {
    return SUPERBLOCK_START + (block_idx - 1) * _ext2_get_block_len(dev);
}

driver_desc_t _ext2_driver_info() {
    driver_desc_t fs_desc;
    fs_desc.type = DRIVER_FILE_SYSTEM;
    fs_desc.auto_start = false;
    fs_desc.is_device_driver = false;
    fs_desc.is_device_needed = false;
    fs_desc.is_driver_needed = false;
    fs_desc.functions[DRIVER_FILE_SYSTEM_RECOGNIZE] = ext2_recognize_drive;
    fs_desc.functions[DRIVER_FILE_SYSTEM_OPEN] = ext2_open;
    fs_desc.functions[DRIVER_FILE_SYSTEM_READ] = ext2_read;
    fs_desc.functions[DRIVER_FILE_SYSTEM_WRITE] = ext2_write;
    fs_desc.functions[DRIVER_FILE_SYSTEM_CREATE_DIR] = ext2_stub;
    fs_desc.functions[DRIVER_FILE_SYSTEM_LOOKUP_DIR] = ext2_stub;
    fs_desc.functions[DRIVER_FILE_SYSTEM_REMOVE_DIR] = ext2_stub;
    fs_desc.functions[DRIVER_FILE_SYSTEM_WRITE_FILE] = ext2_stub;
    fs_desc.functions[DRIVER_FILE_SYSTEM_READ_FILE] = ext2_stub;
    fs_desc.functions[DRIVER_FILE_SYSTEM_REMOVE_FILE] = ext2_stub;
    fs_desc.functions[DRIVER_FILE_SYSTEM_EJECT_DEVICE] = ext2_stub;
    return fs_desc;
}

void ext2_install() {
    driver_install(_ext2_driver_info());
}

// TODO return int
bool ext2_recognize_drive(vfs_device_t *dev) {
    superblock_t *superblock = (superblock_t*)kmalloc(SUPERBLOCK_LEN);
    _ext2_read_from_dev(dev, (uint8_t*)superblock, SUPERBLOCK_START, SUPERBLOCK_LEN);
    
    if (superblock->magic != 0xEF53) {
        return false;
    }
    if (superblock->rev_level != 0) {
        return false;
    }

    _ext2_superblocks[dev->dev.id] = superblock;

    // FIXME: for now we consider that we have at max 5 groups
    uint32_t group_table_len = 5 * GROUP_LEN;
    group_desc_t *group_table = (group_desc_t*)kmalloc(group_table_len);
    _ext2_read_from_dev(dev, (uint8_t*)group_table, _ext2_get_block_offset(dev, 2), group_table_len);

    _ext2_group_tables[dev->dev.id] = group_table;

    return true;
}

void ext2_stub() {
    printf("EXT2\n");
}

int ext2_read_inode(vfs_device_t *dev, uint32_t inode_id, inode_t *inode) {
    uint32_t inodes_per_group = ext2_get_superblock()->inodes_per_group;
    uint32_t holder_group = (inode_id - 1) / inodes_per_group;
    uint32_t pos_inside_group = (inode_id - 1) % inodes_per_group;
    uint32_t inode_start = _ext2_get_block_offset(dev, _ext2_group_tables[dev->dev.id][holder_group].inode_table) + (pos_inside_group * INODE_LEN);
    _ext2_read_from_dev(dev, (uint8_t*)inode, inode_start, INODE_LEN);
    return 0; 
}

int ext2_has_in_dir(vfs_device_t *dev, uint32_t block_idx, const char *path, uint32_t *found_inode_idx) {
    if (block_idx == 0) {
        return -1;
    }

    uint8_t *tmp_buf = (uint8_t*)kmalloc(_ext2_get_block_len(dev));
    _ext2_read_from_dev(dev, tmp_buf, _ext2_get_block_offset(dev, block_idx), _ext2_get_block_len(dev));
    dir_entry_t* start_of_entry = (dir_entry_t*)tmp_buf;
    for (;;) {
        if (start_of_entry->inode == 0) {
            kfree(tmp_buf);
            return -1;
        }
        // checking name of this entry
        bool is_name_correct = true;
        for (int i = 0; i < start_of_entry->name_len; i++) {
            is_name_correct &= (path[i] == *((char*)start_of_entry+8+i));
            
        }
        is_name_correct &= (path[start_of_entry->name_len] == '\0' || path[start_of_entry->name_len] == '/');
        
        if (is_name_correct) {
            *found_inode_idx = start_of_entry->inode;
            kfree(tmp_buf);
            return 0;
        }
        
        start_of_entry = (dir_entry_t*)((uint32_t)start_of_entry + start_of_entry->rec_len);
        if ((uint32_t)start_of_entry >= (uint32_t)tmp_buf + _ext2_get_block_len(dev)) {
            kfree(tmp_buf);
            return -1;
        }
    }
    kfree(tmp_buf);
    return -2;
}

int ext2_scan_dir(vfs_device_t *dev, inode_t inode, const char *path, uint32_t *res_inode_idx) {
    uint32_t nxt_inode_index;
    uint32_t path_offset = 0;
    for (;;) {
new_inode:
        for (int i = 0; i < 12; i++) {
            if (inode.block[i] != 0) {
                if (ext2_has_in_dir(dev, inode.block[i], &path[path_offset], &nxt_inode_index) == 0) {
                    for (; path[path_offset] != '/'; path_offset++) {
                        if (path[path_offset] == '\0') {
                            *res_inode_idx = nxt_inode_index;
                            return 0;
                        }
                    }
                    path_offset++;
                    ext2_read_inode(dev, nxt_inode_index, &inode);
                    goto new_inode;
                }
            }
        }
    }
    return -1;
}

int ext2_get_inode_idx(vfs_device_t *dev, const char *path, uint32_t *file_inode) {
    inode_t root_inode;
    ext2_read_inode(dev, 2, &root_inode);

    int res = -1;
    if (path[0] == '/') {
        res = ext2_scan_dir(dev, root_inode, &path[1], file_inode);
    } else {
        res = ext2_scan_dir(dev, root_inode, path, file_inode);
    }
    
    return res;
}

int ext2_open(vfs_device_t *dev, const char *path, file_descriptor_t *fd) {
    uint32_t inode_idx;
    inode_t inode;
    // while (1) {}
    if (ext2_get_inode_idx(dev, path, &inode_idx) < 0) {
        // perror("[Ext2]: can't find inode");
        return -1;
    }
    // while (1) {}
    if (ext2_read_inode(dev, inode_idx, &inode) < 0) {
        // perror("[Ext2]: can't load inode");
        return -1;
    }
    fd->mode = inode.mode;
    fd->atime = inode.atime;
    fd->ctime = inode.ctime;
    fd->mtime = inode.mtime;
    fd->mode = inode.mode;
    fd->size = inode.size;
    fd->dev_id = dev->dev.id;
    fd->inode_idx = inode_idx;
    return 0;
}

uint32_t _ext2_get_block_of_file_lev0(vfs_device_t *dev, uint32_t cur_block, uint32_t file_block_index) {
    uint32_t offset = file_block_index;
    uint32_t res;
    _ext2_read_from_dev(dev, (uint8_t*)&res, _ext2_get_block_offset(dev, cur_block) + offset * 4, 4);
    return res;
}

uint32_t _ext2_get_block_of_file_lev1(vfs_device_t *dev, uint32_t cur_block, uint32_t file_block_index) {
    uint32_t lev_contain = _ext2_get_block_len(dev) / 4;
    uint32_t offset = file_block_index / lev_contain;
    uint32_t offset_inner = file_block_index % lev_contain;
    uint32_t res;
    _ext2_read_from_dev(dev, (uint8_t*)&res, _ext2_get_block_offset(dev, cur_block) + offset * 4, 4);
    return _ext2_get_block_of_file_lev0(dev, res, offset_inner);
}

uint32_t _ext2_get_block_of_file_lev2(vfs_device_t *dev, uint32_t cur_block, uint32_t file_block_index) {
    uint32_t block_len = _ext2_get_block_len(dev) / 4;
    uint32_t lev_contain = block_len * block_len;
    uint32_t offset = file_block_index / lev_contain;
    uint32_t offset_inner = file_block_index % lev_contain;
    uint32_t res;
    _ext2_read_from_dev(dev, (uint8_t*)&res, _ext2_get_block_offset(dev, cur_block) + offset * 4, 4);
    return _ext2_get_block_of_file_lev1(dev, res, offset_inner);
}

uint32_t _ext2_get_block_of_file_lev3(vfs_device_t *dev, uint32_t cur_block, uint32_t file_block_index) {
     uint32_t block_len = _ext2_get_block_len(dev) / 4;
    uint32_t lev_contain = block_len * block_len * block_len;
    uint32_t offset = file_block_index / lev_contain;
    uint32_t offset_inner = file_block_index % lev_contain;
    uint32_t res;
    _ext2_read_from_dev(dev, (uint8_t*)&res, _ext2_get_block_offset(dev, cur_block) + offset * 4, 4);
    return _ext2_get_block_of_file_lev2(dev, res, offset_inner);
}

// TODO think of more effecient way
uint32_t _ext2_get_block_of_file(vfs_device_t *dev, inode_t *inode, uint32_t file_block_index) {
    uint32_t block_len = _ext2_get_block_len(dev) / 4;
    if (file_block_index < 12) {
        return inode->block[file_block_index];
    } else if (file_block_index < 12 + block_len) { // single indirect
        return _ext2_get_block_of_file_lev0(dev, inode->block[12], file_block_index - 12);
    } else if (file_block_index < 12 + block_len + block_len * block_len) { // double indirect
        return _ext2_get_block_of_file_lev1(dev, inode->block[13], file_block_index - 12 - block_len);
    } else { // triple indirect
        return _ext2_get_block_of_file_lev0(dev, inode->block[14], file_block_index - (12 + block_len + block_len * block_len));
    }
}


int ext2_read(vfs_device_t *dev, file_descriptor_t *fd, uint8_t* buf, uint32_t start, uint32_t len) {
    inode_t inode;
    ext2_read_inode(dev, fd->inode_idx, &inode);

    const uint32_t block_len = _ext2_get_block_len(dev);
    uint32_t start_block_index = start / block_len;
    uint32_t end_block_index = (start + len) / block_len;
    uint32_t read_offset = start % block_len;
    uint32_t already_read = 0; 

    for (uint32_t block_index = start_block_index; block_index <= end_block_index; block_index++) {
        uint32_t data_block_index = _ext2_get_block_of_file(dev, &inode, block_index);
        _ext2_read_from_dev(dev, buf+already_read, _ext2_get_block_offset(dev, data_block_index) + read_offset, MIN(len, block_len - read_offset));
        printf("Reading "); printd(already_read); printf("\n");
        len -= MIN(len, block_len - read_offset);
        already_read += MIN(len, block_len - read_offset);
        read_offset = 0;
    }
    if (len != 0) {
        return -1;
    }

    printf("RET 0\n");
    return 0;
}

int ext2_write(vfs_device_t *dev, file_descriptor_t *fd, uint8_t* buf, uint32_t start, uint32_t len) {
    inode_t inode;
    ext2_read_inode(dev, fd->inode_idx, &inode);

    const uint32_t block_len = _ext2_get_block_len(dev);
    uint32_t start_block_index = start / block_len;
    uint32_t end_block_index = (start + len) / block_len;
    uint32_t write_offset = start % block_len;
    uint32_t already_written = start % block_len;
    
    for (uint32_t block_index = start_block_index; block_index <= end_block_index; block_index++) {
        uint32_t data_block_index = _ext2_get_block_of_file(dev, &inode, block_index);
        _ext2_write_to_dev(dev, buf+already_written, _ext2_get_block_offset(dev, data_block_index) + write_offset, MIN(len, block_len - write_offset));
        printf("Writing "); printd (MIN(len, block_len - write_offset)); printf("\n");
        len -= MIN(len, block_len - write_offset);
        already_written += MIN(len, block_len - write_offset);
        write_offset = 0;

    }
    if (len != 0) {
        return -1;
    }

    printf("RET 0\n");
    return 0;
}
