#include <fs/ext2/ext2.h>
#include <mem/malloc.h>

static superblock_t *_ext2_superblocks[MAX_DEVICES_COUNT];

driver_desc_t _ext2_driver_info();
void _ext2_read_superblock();

void _ext2_read_superblock(vfs_device_t *dev, superblock_t *buf) {
    void (*rd)(device_t *d, uint32_t s, uint8_t *r) =
        drivers[dev->dev.driver_id].driver_desc.functions[DRIVER_STORAGE_READ];
    rd(&dev->dev, 2, (uint8_t*)buf);
    // rd(&dev->dev, 1024+512, ((uint8_t*)buf)+512);
}

driver_desc_t _ext2_driver_info() {
    driver_desc_t fs_desc;
    fs_desc.type = DRIVER_FILE_SYSTEM;
    fs_desc.auto_start = false;
    fs_desc.is_device_driver = false;
    fs_desc.is_device_needed = false;
    fs_desc.is_driver_needed = false;
    fs_desc.functions[DRIVER_FILE_SYSTEM_RECOGNIZE] = ext2_recognize_drive;
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

// TODO better to return int
bool ext2_recognize_drive(vfs_device_t *dev) {
    superblock_t *superblock = (superblock_t*)kmalloc(1024);
    _ext2_read_superblock(dev, superblock);
    // printh(superblock->magic);
    // while (1) {}
    if (superblock->magic != 0xEF53) {
        return false;
    }
    if (superblock->rev_level != 0) {
        return false;
    }

    _ext2_superblocks[dev->dev.id] = superblock;

    printf("EXT2\n");
    while (1) {}
    // start setup process
    return true;
}

void ext2_stub() {
    printf("EXT2\n");
}