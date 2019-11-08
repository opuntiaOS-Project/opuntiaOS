#include <fs/fat16/fat16.h>
#include <drivers/display.h>
#include <mem/malloc.h>

// Private

fat16_drive_desc_t _fat16_driver[VFS_MAX_DEV_COUNT];
uint8_t _fat16_drives_count = 0;

fs_desc_t _fat16_fs_desc();
void _fat16_read(vfs_device_t *t_vfs_dev, uint32_t t_addr, uint8_t *t_buf);
void _fat16_write(vfs_device_t *t_vfs_dev, uint32_t t_addr, uint8_t *t_buf, uint32_t t_buf_size);
uint8_t _fat16_load_fat(vfs_device_t *t_vfs_dev, fat16_drive_desc_t *t_drive);
uint8_t _fat16_register_drive(vfs_device_t *t_vfs_dev, uint8_t *t_buf);

fs_desc_t _fat16_fs_desc() {
    fs_desc_t fat16;
    fat16.recognize = fat16_recognize;
    return fat16;
}

void _fat16_read(vfs_device_t *t_vfs_dev, uint32_t t_addr, uint8_t *t_buf) {
    printf("DRIVER ID:"); printd(t_vfs_dev->dev.driver_id);
    void (*rd)(device_t *d, uint32_t s, uint8_t *r) =
        drivers[t_vfs_dev->dev.driver_id].driver_desc.functions[DRIVER_STORAGE_READ];
    rd(&t_vfs_dev->dev, t_addr, t_buf);
}

void _fat16_write(vfs_device_t *t_vfs_dev, uint32_t t_addr, uint8_t *t_buf, uint32_t t_buf_size) {
    void (*wr)(device_t *d, uint32_t s, uint8_t *r, uint32_t bs) =
        drivers[t_vfs_dev->dev.driver_id].driver_desc.functions[DRIVER_STORAGE_WRITE];
    wr(&t_vfs_dev->dev, t_addr, t_buf, t_buf_size);
}

uint8_t _fat16_load_fat(vfs_device_t *t_vfs_dev, fat16_drive_desc_t *t_drive) {
    // Todo check Fats if multiple
    uint8_t data[512];
    uint32_t fat_size_bytes = t_drive->sectors_per_fat * t_drive->bytes_per_sector;
    t_drive->fat_ptr = (uint8_t*)kmalloc(fat_size_bytes);
    for (uint16_t sector_id = 0; sector_id < t_drive->sectors_per_fat; sector_id++) {
        _fat16_read(t_vfs_dev, t_drive->start_of_fats + sector_id, data);
        for (uint16_t i_copy = 0; i_copy < t_drive->bytes_per_sector; i_copy++) {
            uint32_t offset = sector_id * t_drive->bytes_per_sector + i_copy;
            t_drive->fat_ptr[offset] = data[i_copy];
        }
    }
}

uint8_t _fat16_register_drive(vfs_device_t *t_vfs_dev, uint8_t *t_buf) {
    fat16_drive_desc_t new_drive;
    new_drive.bytes_per_sector = t_buf[0x0c] * 0x100 + t_buf[0x0b];
    new_drive.sectors_per_cluster = 1; // data[0x0d]
    new_drive.bytes_per_cluster = new_drive.bytes_per_sector * new_drive.sectors_per_cluster;
    new_drive.reserved_sectors = t_buf[0x0f] * 0x100 + t_buf[0x0e];
    new_drive.start_of_fats = new_drive.reserved_sectors;
    new_drive.number_of_fats = t_buf[0x10];
    new_drive.sectors_per_fat = t_buf[0x17] * 0x100 + t_buf[0x16];
    new_drive.root_entries = t_buf[0x12] * 0x100 + t_buf[0x11];
    new_drive.root_dir_start = new_drive.start_of_fats + new_drive.number_of_fats * new_drive.sectors_per_fat;
    new_drive.data_seg_start = new_drive.root_dir_start + (new_drive.root_entries * 32) / new_drive.bytes_per_sector;


    _fat16_load_fat(t_vfs_dev, &new_drive);

    _fat16_driver[_fat16_drives_count++] = new_drive;
    return _fat16_drives_count - 1;
}

// Public

void fat16_install() {
    vfs_add_fs(_fat16_fs_desc());
}

// Recognize if it's fat16 drive and if it's fat16 drive, will registered that
// drive (Collect info, load fat into ram, etc.)
bool fat16_recognize(vfs_device_t *t_vfs_dev) {
    uint8_t res[512];
    _fat16_read(t_vfs_dev, 0, res);
    char fat16Signature[] = {'F', 'A', 'T', '1', '6', 0x20, 0x20, 0x20};
    bool isSignatureCorrect = true;
    for (uint16_t i = 0x36; i < 0x36 + 8; i++) {
        isSignatureCorrect &= (fat16Signature[i - 0x36] == res[i]);
    }
    if (isSignatureCorrect) {
        uint8_t translate_id = _fat16_register_drive(t_vfs_dev, res);
        t_vfs_dev->translate_id = translate_id;
    }
    return isSignatureCorrect;
}
