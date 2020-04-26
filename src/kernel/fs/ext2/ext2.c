#include <fs/ext2/ext2.h>
#include <mem/malloc.h>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define SUPERBLOCK _ext2_superblocks[dev->dev.id]
#define GROUP_TABLES _ext2_group_tables[dev->dev.id]
#define BLOCK_LEN (1 << (SUPERBLOCK->log_block_size + 10))
#define NORM_FILENAME(x) (x + ((4 - (x & 0b11)) & 0b11))

static superblock_t* _ext2_superblocks[MAX_DEVICES_COUNT];
static group_desc_t* _ext2_group_tables[MAX_DEVICES_COUNT];

driver_desc_t _ext2_driver_info();

/* TOOLS */
void _ext2_read_from_dev(vfs_device_t* dev, uint8_t* buf, uint32_t start, uint32_t len);
void _ext2_write_to_dev(vfs_device_t* dev, uint8_t* buf, uint32_t start, uint32_t len);
uint32_t _ext2_get_groups_cnt(vfs_device_t* dev);
uint32_t _ext2_get_block_offset(vfs_device_t* dev, uint32_t block_index);

/**
 * DRIVE FUNCTIONS
 */

void _ext2_read_from_dev(vfs_device_t* dev, uint8_t* buf, uint32_t start, uint32_t len)
{
    void (*read)(device_t * d, uint32_t s, uint8_t * r) = drivers[dev->dev.driver_id].driver_desc.functions[DRIVER_STORAGE_READ];
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

void _ext2_write_to_dev(vfs_device_t* dev, uint8_t* buf, uint32_t start, uint32_t len)
{
    void (*read)(device_t * d, uint32_t s, uint8_t * r) = drivers[dev->dev.driver_id].driver_desc.functions[DRIVER_STORAGE_READ];
    void (*write)(device_t * d, uint32_t s, uint8_t * r, uint32_t siz) = drivers[dev->dev.driver_id].driver_desc.functions[DRIVER_STORAGE_WRITE];
    int already_written = 0;
    uint8_t tmp_buf[512];
    while (len != 0) {
        uint32_t sector = start / 512;
        uint32_t start_offset = start % 512;
        if (start_offset != 0 || len < 512) {
            read(&dev->dev, sector, tmp_buf);
        }
        for (int i = 0; i < MIN(512 - start_offset, len); i++) {
            tmp_buf[start_offset + i] = buf[already_written++];
        }
        write(&dev->dev, sector, tmp_buf, 512);
        len -= MIN(512 - start_offset, len);
        start += MIN(512 - start_offset, len);
    }
}

bool _ext2_bitmap_get(uint8_t* bitmap, uint32_t index)
{
    return (bitmap[index / 8] >> (index % 8)) & 1;
}

void _ext2_bitmap_set(uint8_t* bitmap, uint32_t index, bool value)
{
    bitmap[index / 8] |= ((value & 1) << (index % 8));
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
    fs_desc.functions[DRIVER_FILE_SYSTEM_OPEN] = ext2_open;
    fs_desc.functions[DRIVER_FILE_SYSTEM_READ] = ext2_read;
    fs_desc.functions[DRIVER_FILE_SYSTEM_WRITE] = ext2_write;
    fs_desc.functions[DRIVER_FILE_SYSTEM_MKDIR] = ext2_mkdir;
    fs_desc.functions[DRIVER_FILE_SYSTEM_CREATE_DIR] = ext2_stub;
    fs_desc.functions[DRIVER_FILE_SYSTEM_LOOKUP_DIR] = ext2_stub;
    fs_desc.functions[DRIVER_FILE_SYSTEM_REMOVE_DIR] = ext2_stub;
    fs_desc.functions[DRIVER_FILE_SYSTEM_WRITE_FILE] = ext2_stub;
    fs_desc.functions[DRIVER_FILE_SYSTEM_READ_FILE] = ext2_stub;
    fs_desc.functions[DRIVER_FILE_SYSTEM_REMOVE_FILE] = ext2_stub;
    fs_desc.functions[DRIVER_FILE_SYSTEM_EJECT_DEVICE] = ext2_stub;
    return fs_desc;
}

void ext2_install()
{
    driver_install(_ext2_driver_info());
}

// TODO return int
bool ext2_recognize_drive(vfs_device_t* dev)
{
    superblock_t* superblock = (superblock_t*)kmalloc(SUPERBLOCK_LEN);
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
    group_desc_t* group_table = (group_desc_t*)kmalloc(group_table_len);
    _ext2_read_from_dev(dev, (uint8_t*)group_table, _ext2_get_block_offset(dev, 2), group_table_len);

    _ext2_group_tables[dev->dev.id] = group_table;

    return true;
}

void ext2_stub()
{
    printf("EXT2\n");
}

/**
 * BLOCK FUNCTIONS
 */

uint32_t _ext2_get_block_offset(vfs_device_t* dev, uint32_t block_index)
{
    return SUPERBLOCK_START + (block_index - 1) * BLOCK_LEN;
}

uint32_t _ext2_get_block_of_inode_lev0(vfs_device_t* dev, uint32_t cur_block, uint32_t inode_block_index)
{
    uint32_t offset = inode_block_index;
    uint32_t res;
    _ext2_read_from_dev(dev, (uint8_t*)&res, _ext2_get_block_offset(dev, cur_block) + offset * 4, 4);
    return res;
}

uint32_t _ext2_get_block_of_inode_lev1(vfs_device_t* dev, uint32_t cur_block, uint32_t inode_block_index)
{
    uint32_t lev_contain = BLOCK_LEN / 4;
    uint32_t offset = inode_block_index / lev_contain;
    uint32_t offset_inner = inode_block_index % lev_contain;
    uint32_t res;
    _ext2_read_from_dev(dev, (uint8_t*)&res, _ext2_get_block_offset(dev, cur_block) + offset * 4, 4);
    return res ? _ext2_get_block_of_inode_lev0(dev, res, offset_inner) : 0;
}

uint32_t _ext2_get_block_of_inode_lev2(vfs_device_t* dev, uint32_t cur_block, uint32_t inode_block_index)
{
    uint32_t block_len = BLOCK_LEN / 4;
    uint32_t lev_contain = block_len * block_len;
    uint32_t offset = inode_block_index / lev_contain;
    uint32_t offset_inner = inode_block_index % lev_contain;
    uint32_t res;
    _ext2_read_from_dev(dev, (uint8_t*)&res, _ext2_get_block_offset(dev, cur_block) + offset * 4, 4);
    return res ? _ext2_get_block_of_inode_lev1(dev, res, offset_inner) : 0;
}

// FIXME: think of more effecient way
uint32_t _ext2_get_block_of_inode(vfs_device_t* dev, inode_t* inode, uint32_t inode_block_index)
{
    uint32_t block_len = BLOCK_LEN / 4;
    if (inode_block_index < 12) {
        return inode->block[inode_block_index];
    } else if (inode_block_index < 12 + block_len) { // single indirect
        return _ext2_get_block_of_inode_lev0(dev, inode->block[12], inode_block_index - 12);
    } else if (inode_block_index < 12 + block_len + block_len * block_len) { // double indirect
        return _ext2_get_block_of_inode_lev1(dev, inode->block[13], inode_block_index - 12 - block_len);
    } else { // triple indirect
        return _ext2_get_block_of_inode_lev2(dev, inode->block[14], inode_block_index - (12 + block_len + block_len * block_len));
    }
}

int _ext2_set_block_of_inode_lev0(vfs_device_t* dev, uint32_t cur_block, uint32_t inode_block_index, uint32_t val)
{
    uint32_t offset = inode_block_index;
    _ext2_write_to_dev(dev, (uint8_t*)&val, _ext2_get_block_offset(dev, cur_block) + offset * 4, 4);
    return 0;
}

int _ext2_set_block_of_inode_lev1(vfs_device_t* dev, uint32_t cur_block, uint32_t inode_block_index, uint32_t val)
{
    uint32_t lev_contain = BLOCK_LEN / 4;
    uint32_t offset = inode_block_index / lev_contain;
    uint32_t offset_inner = inode_block_index % lev_contain;
    uint32_t res;
    _ext2_read_from_dev(dev, (uint8_t*)&res, _ext2_get_block_offset(dev, cur_block) + offset * 4, 4);
    return res ? _ext2_set_block_of_inode_lev0(dev, res, offset_inner, val) : -1;
}

int _ext2_set_block_of_inode_lev2(vfs_device_t* dev, uint32_t cur_block, uint32_t inode_block_index, uint32_t val)
{
    uint32_t block_len = BLOCK_LEN / 4;
    uint32_t lev_contain = block_len * block_len;
    uint32_t offset = inode_block_index / lev_contain;
    uint32_t offset_inner = inode_block_index % lev_contain;
    uint32_t res;
    _ext2_read_from_dev(dev, (uint8_t*)&res, _ext2_get_block_offset(dev, cur_block) + offset * 4, 4);
    return res ? _ext2_set_block_of_inode_lev1(dev, res, offset_inner, val) : -1;
}

// FIXME: think of more effecient way
int _ext2_set_block_of_inode(vfs_device_t* dev, inode_t* inode, uint32_t inode_block_index, uint32_t val)
{
    uint32_t block_len = BLOCK_LEN / 4;
    if (inode_block_index < 12) {
        inode->block[inode_block_index] = val;
        return 0;
    } else if (inode_block_index < 12 + block_len) { // single indirect
        return _ext2_set_block_of_inode_lev0(dev, inode->block[12], inode_block_index - 12, val);
    } else if (inode_block_index < 12 + block_len + block_len * block_len) { // double indirect
        return _ext2_set_block_of_inode_lev1(dev, inode->block[13], inode_block_index - 12 - block_len, val);
    } else { // triple indirect
        return _ext2_set_block_of_inode_lev2(dev, inode->block[14], inode_block_index - (12 + block_len + block_len * block_len), val);
    }
}

static int ext2_find_free_block(vfs_device_t* dev, uint32_t* block_index, uint32_t group_index)
{
    uint8_t* block_bitmap = (uint8_t*)kmalloc(BLOCK_LEN);
    _ext2_read_from_dev(dev, block_bitmap, _ext2_get_block_offset(dev, GROUP_TABLES[group_index].block_bitmap), BLOCK_LEN);

    for (uint32_t off = 0; off < BLOCK_LEN; off++) {
        if (!_ext2_bitmap_get(block_bitmap, off)) {
            *block_index = SUPERBLOCK->blocks_per_group * group_index + off + 1;
            _ext2_bitmap_set(block_bitmap, off, 1);
            _ext2_write_to_dev(dev, block_bitmap, _ext2_get_block_offset(dev, GROUP_TABLES[group_index].block_bitmap), BLOCK_LEN);
            kfree(block_bitmap);
            return 0;
        }
    }
    kfree(block_bitmap);
    return -1;
}

static int ext2_allocate_block(vfs_device_t* dev, uint32_t* block_index, uint32_t pref_group)
{
    // TODO: change group count to real value
    uint32_t groups_cnt = 3;
    for (int i = 0; i < groups_cnt; i++) {
        if (GROUP_TABLES[(pref_group + i) % groups_cnt].free_inodes_count) {
            if (ext2_find_free_block(dev, block_index, pref_group) == 0) {
                return 0;
            }
        }
    }
    return -1;
}

/**
 * Returns allocated block in @block_index
 */
static int ext2_allocate_block_for_inode(vfs_device_t* dev, inode_t* inode, uint32_t pref_group, uint32_t* block_index)
{
    if (ext2_allocate_block(dev, block_index, pref_group) == 0) {
        uint32_t blocks_per_inode = inode->blocks / (BLOCK_LEN / 512);
        if (_ext2_set_block_of_inode(dev, inode, blocks_per_inode, *block_index) == 0) {
            inode->blocks += BLOCK_LEN / 512;
            return 0;
        }
    }
    return -1;
}

/**
 * INODE FUNCTIONS
 */

int ext2_read_inode(vfs_device_t* dev, uint32_t inode_index, inode_t* inode)
{
    uint32_t inodes_per_group = SUPERBLOCK->inodes_per_group;
    uint32_t holder_group = (inode_index - 1) / inodes_per_group;
    uint32_t pos_inside_group = (inode_index - 1) % inodes_per_group;
    uint32_t inode_start = _ext2_get_block_offset(dev, _ext2_group_tables[dev->dev.id][holder_group].inode_table) + (pos_inside_group * INODE_LEN);
    _ext2_read_from_dev(dev, (uint8_t*)inode, inode_start, INODE_LEN);
    return 0;
}

int ext2_write_inode(vfs_device_t* dev, uint32_t inode_index, inode_t* inode)
{
    uint32_t inodes_per_group = SUPERBLOCK->inodes_per_group;
    uint32_t holder_group = (inode_index - 1) / inodes_per_group;
    uint32_t pos_inside_group = (inode_index - 1) % inodes_per_group;
    uint32_t inode_start = _ext2_get_block_offset(dev, _ext2_group_tables[dev->dev.id][holder_group].inode_table) + (pos_inside_group * INODE_LEN);
    _ext2_write_to_dev(dev, (uint8_t*)inode, inode_start, INODE_LEN);
    return 0;
}

int ext2_find_free_inode(vfs_device_t* dev, uint32_t* inode_index, uint32_t group_index)
{
    uint8_t* inode_bitmap = (uint8_t*)kmalloc(BLOCK_LEN);
    _ext2_read_from_dev(dev, inode_bitmap, _ext2_get_block_offset(dev, GROUP_TABLES[group_index].inode_bitmap), BLOCK_LEN);

    for (uint32_t off = 0; off < BLOCK_LEN; off++) {
        if (!_ext2_bitmap_get(inode_bitmap, off)) {
            *inode_index = SUPERBLOCK->inodes_per_group * group_index + off + 1;
            _ext2_bitmap_set(inode_bitmap, off, 1);
            _ext2_write_to_dev(dev, inode_bitmap, _ext2_get_block_offset(dev, GROUP_TABLES[group_index].inode_bitmap), BLOCK_LEN);
            kfree(inode_bitmap);
            return 0;
        }
    }
    kfree(inode_bitmap);
    return -1;
}

int ext2_allocate_inode(vfs_device_t* dev, uint32_t* inode_index, uint32_t pref_group)
{
    // TODO: change group count to real value
    uint32_t groups_cnt = 3;
    for (int i = 0; i < groups_cnt; i++) {
        if (GROUP_TABLES[(pref_group + i) % groups_cnt].free_inodes_count) {
            if (ext2_find_free_inode(dev, inode_index, pref_group) == 0) {
                return 0;
            }
        }
    }
    return -1;
}

/**
 * DIR FUNCTIONS
 */

// TODO: currently only link version is supported.
// TODO: add cache.
int _ext2_scan_dir_block(vfs_device_t* dev, uint32_t block_index, const char* path, uint32_t* found_inode_index)
{
    if (block_index == 0) {
        return -1;
    }

    uint8_t* tmp_buf = (uint8_t*)kmalloc(BLOCK_LEN);
    _ext2_read_from_dev(dev, tmp_buf, _ext2_get_block_offset(dev, block_index), BLOCK_LEN);
    dir_entry_t* start_of_entry = (dir_entry_t*)tmp_buf;
    for (;;) {
        if (start_of_entry->inode == 0) {
            kfree(tmp_buf);
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
            kfree(tmp_buf);
            return 0;
        }

        start_of_entry = (dir_entry_t*)((uint32_t)start_of_entry + start_of_entry->rec_len);
        if ((uint32_t)start_of_entry >= (uint32_t)tmp_buf + BLOCK_LEN) {
            kfree(tmp_buf);
            return -1;
        }
    }
    kfree(tmp_buf);
    return -2;
}

int ext2_dir_find_child(vfs_device_t* dev, inode_t inode, const char* name, uint32_t* res_inode_index)
{
    uint32_t block_per_dir = inode.blocks / (BLOCK_LEN / 512);
    uint32_t found_inode = 0;
    for (int block_index = 0; block_index < block_per_dir; block_index++) {
        uint32_t data_block_index = _ext2_get_block_of_inode(dev, &inode, block_index);
        if (_ext2_scan_dir_block(dev, data_block_index, name, res_inode_index) == 0) {
            return 0;
        }
    }
    return -1;
}

int _ext2_add_first_entry_to_dir_block(vfs_device_t* dev, uint32_t block_index, uint32_t added_inode_index, const char* filename)
{
    if (block_index == 0) {
        return -1;
    }

    uint32_t filename_real_len = strlen(filename);
    uint32_t filename_len = NORM_FILENAME(filename_real_len);
    uint32_t min_rec_len = 8 + filename_len;
    dir_entry_t new_entry;

    uint8_t* tmp_buf = (uint8_t*)kmalloc(DIR_ENTRY_LEN);
    _ext2_read_from_dev(dev, tmp_buf, _ext2_get_block_offset(dev, block_index), DIR_ENTRY_LEN);
    dir_entry_t* start_of_entry = (dir_entry_t*)tmp_buf;
    new_entry.inode = added_inode_index;
    new_entry.rec_len = BLOCK_LEN;
    new_entry.name_len = filename_real_len;
    memcpy((void*)start_of_entry, (void*)&new_entry, 8);
    memcpy((void*)((uint32_t)start_of_entry + 8), (void*)filename, filename_real_len);
    memset((void*)((uint32_t)start_of_entry + 8 + filename_real_len), 0, filename_len - filename_real_len);
    _ext2_write_to_dev(dev, tmp_buf, _ext2_get_block_offset(dev, block_index), DIR_ENTRY_LEN);
    kfree(tmp_buf);
    return 0;
}

int _ext2_add_to_dir_block(vfs_device_t* dev, uint32_t block_index, uint32_t added_inode_index, const char* filename)
{
    if (block_index == 0) {
        return -1;
    }

    uint32_t filename_real_len = strlen(filename);
    uint32_t filename_len = NORM_FILENAME(filename_real_len);
    uint32_t min_rec_len = 8 + filename_len;
    dir_entry_t new_entry;

    uint8_t* tmp_buf = (uint8_t*)kmalloc(BLOCK_LEN);
    _ext2_read_from_dev(dev, tmp_buf, _ext2_get_block_offset(dev, block_index), BLOCK_LEN);
    dir_entry_t* start_of_entry = (dir_entry_t*)tmp_buf;
    dir_entry_t* start_of_new_entry;

    if (start_of_entry->inode == 0) {
        new_entry.inode = added_inode_index;
        new_entry.rec_len = BLOCK_LEN;
        new_entry.name_len = filename_real_len;
        start_of_new_entry = start_of_entry;
        goto update_res;
    }

    for (;;) {
        uint32_t cur_filename_len = NORM_FILENAME(start_of_entry->name_len);
        uint32_t cur_rec_len = 8 + cur_filename_len;

        // We have enough place to put both records
        if (start_of_entry->rec_len >= cur_rec_len + min_rec_len) {
            new_entry.inode = added_inode_index;
            new_entry.rec_len = start_of_entry->rec_len - cur_rec_len;
            new_entry.name_len = filename_real_len;
            start_of_new_entry = (dir_entry_t*)((uint32_t)start_of_entry + cur_rec_len);
            start_of_entry->rec_len = cur_rec_len;
            goto update_res;
        }

        start_of_entry = (dir_entry_t*)((uint32_t)start_of_entry + start_of_entry->rec_len);
        if ((uint32_t)start_of_entry >= (uint32_t)tmp_buf + BLOCK_LEN) {
            kfree(tmp_buf);
            return -1;
        }
    }

update_res:
    memcpy((void*)start_of_new_entry, (void*)&new_entry, 8);
    memcpy((void*)((uint32_t)start_of_new_entry + 8), (void*)filename, filename_real_len);
    memset((void*)((uint32_t)start_of_new_entry + 8 + filename_real_len), 0, filename_len - filename_real_len);
    _ext2_write_to_dev(dev, tmp_buf, _ext2_get_block_offset(dev, block_index), BLOCK_LEN);
    kfree(tmp_buf);
    return 0;
}

int ext2_add_child(vfs_device_t* dev, inode_t* inode, uint32_t child_inode_index, const char* with_name)
{
    inode_t child_inode;
    uint32_t block_index;
    uint32_t blocks_per_dir = inode->blocks / (BLOCK_LEN / 512);

    for (int i = 0; i < blocks_per_dir; i++) {
        if (block_index = _ext2_get_block_of_inode(dev, inode, i)) {
            if (_ext2_add_to_dir_block(dev, block_index, child_inode_index, with_name) == 0) {
                goto update_inode;
            }
        }
    }

    // FIXME: group
    uint32_t new_block_index;
    if (ext2_allocate_block_for_inode(dev, inode, 0, &new_block_index) == 0) {
        if (_ext2_add_first_entry_to_dir_block(dev, new_block_index, child_inode_index, with_name) == 0) {
            goto update_inode;
        }
    }

    return -1;
update_inode:
    ext2_read_inode(dev, child_inode_index, &child_inode);
    child_inode.links_count++;
    ext2_write_inode(dev, child_inode_index, &child_inode);
    return 0;
}

int ext2_setup_dir(vfs_device_t* dev, uint32_t inode_index, uint32_t parent_inode_index)
{
    inode_t inode;
    if (ext2_read_inode(dev, inode_index, &inode) < 0) {
        return -1;
    }
    inode.mode = EXT2_S_IFDIR | EXT2_S_IRUSR | EXT2_S_IWUSR | EXT2_S_IXUSR | EXT2_S_IRGRP | EXT2_S_IXGRP | EXT2_S_IROTH | EXT2_S_IXOTH;
    inode.uid = 0; // FIXME: uid of real user
    inode.links_count = 0;
    inode.blocks = 0;
    if (ext2_add_child(dev, &inode, inode_index, ".") < 0) {
        return -1;
    }
    if (ext2_add_child(dev, &inode, parent_inode_index, "..") < 0) {
        return -1;
    }
    if (ext2_write_inode(dev, inode_index, &inode) < 0) {
        return -1;
    }
    return 0;
}

int ext2_process_inode_by_path(vfs_device_t* dev, inode_t inode, const char* path, uint32_t* res_inode_index)
{
    uint32_t nxt_inode_index;
    uint32_t path_offset = 0;

    for (;;) {
        if (ext2_dir_find_child(dev, inode, &path[path_offset], &nxt_inode_index) == 0) {
            for (; path[path_offset] != '/'; path_offset++) {
                if (path[path_offset] == '\0') {
                    *res_inode_index = nxt_inode_index;
                    return 0;
                }
            }
            path_offset++;
            ext2_read_inode(dev, nxt_inode_index, &inode);
        }
    }
    return -1;
}

int ext2_get_inode_by_path(vfs_device_t* dev, const char* path, inode_t* root_inode, uint32_t* res_inode_index)
{
    // TODO: not sure if I need path[0] == '/' check here.
    inode_t s_inode;
    if (root_inode == 0 || path[0] == '/') {
        root_inode = &s_inode;
        ext2_read_inode(dev, 2, root_inode);
    }

    int res = -1;
    if (path[0] == '/') {
        if (path[1] == '\0') {
            *res_inode_index = 2; // root FIXME!
            return 0;
        }
        res = ext2_process_inode_by_path(dev, *root_inode, &path[1], res_inode_index);
    } else {
        res = ext2_process_inode_by_path(dev, *root_inode, path, res_inode_index);
    }

    return res;
}

// TODO: add support for root
int ext2_open(vfs_device_t* dev, const char* path, file_descriptor_t* fd)
{
    uint32_t inode_index;
    inode_t inode;

    if (ext2_get_inode_by_path(dev, path, 0, &inode_index) < 0) {
        // perror("[Ext2]: can't find inode");
        return -1;
    }
    if (ext2_read_inode(dev, inode_index, &inode) < 0) {
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
    fd->inode_index = inode_index;
    return 0;
}

int ext2_read(vfs_device_t* dev, file_descriptor_t* fd, uint8_t* buf, uint32_t start, uint32_t len)
{
    inode_t inode;
    ext2_read_inode(dev, fd->inode_index, &inode);

    const uint32_t block_len = BLOCK_LEN;
    uint32_t start_block_index = start / block_len;
    uint32_t end_block_index = (start + len) / block_len;
    uint32_t read_offset = start % block_len;
    uint32_t already_read = 0;

    for (uint32_t block_index = start_block_index; block_index <= end_block_index; block_index++) {
        uint32_t data_block_index = _ext2_get_block_of_inode(dev, &inode, block_index);
        _ext2_read_from_dev(dev, buf + already_read, _ext2_get_block_offset(dev, data_block_index) + read_offset, MIN(len, block_len - read_offset));
        len -= MIN(len, block_len - read_offset);
        already_read += MIN(len, block_len - read_offset);
        read_offset = 0;
    }
    if (len != 0) {
        return -1;
    }

    return 0;
}

// FIXME: allocate blocks
int ext2_write(vfs_device_t* dev, file_descriptor_t* fd, uint8_t* buf, uint32_t start, uint32_t len)
{
    inode_t inode;
    ext2_read_inode(dev, fd->inode_index, &inode);

    const uint32_t block_len = BLOCK_LEN;
    uint32_t start_block_index = start / block_len;
    uint32_t end_block_index = (start + len) / block_len;
    uint32_t write_offset = start % block_len;
    uint32_t already_written = start % block_len;

    for (uint32_t block_index = start_block_index; block_index <= end_block_index; block_index++) {
        uint32_t data_block_index = _ext2_get_block_of_inode(dev, &inode, block_index);
        _ext2_write_to_dev(dev, buf + already_written, _ext2_get_block_offset(dev, data_block_index) + write_offset, MIN(len, block_len - write_offset));
        len -= MIN(len, block_len - write_offset);
        already_written += MIN(len, block_len - write_offset);
        write_offset = 0;
    }

    if (len != 0) {
        return -1;
    }

    return 0;
}

int ext2_mkdir(vfs_device_t* dev, file_descriptor_t* fd, const char* name)
{
    inode_t inode;
    if (ext2_read_inode(dev, fd->inode_index, &inode) < 0) {
        // perror("[Ext2]: can't load inode");
        return -1;
    }
    uint32_t dir_inode_index;
    if (ext2_allocate_inode(dev, &dir_inode_index, 0) < 0) {
        return -1;
    }
    if (ext2_setup_dir(dev, dir_inode_index, fd->inode_index) < 0) {
        return -1;
    }
    if (ext2_add_child(dev, &inode, dir_inode_index, name) < 0) {
        return -1;    
    }
    if (ext2_write_inode(dev, fd->inode_index, &inode) < 0) {
        return -1;
    } // Needed here, because add_child can change inode (when allocation new blocks)
    return 0;
}