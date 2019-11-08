#include <fs/fat16/fat16.h>
#include <drivers/display.h>

// Private

fs_desc_t _fat16_fs_desc();
void _fat16_read(vfs_device_t *t_vfs_dev, uint32_t t_addr, uint8_t *t_buf);
void _fat16_write(vfs_device_t *t_vfs_dev, uint32_t t_addr, uint8_t *t_buf, uint32_t t_buf_size);

fs_desc_t _fat16_fs_desc() {
    fs_desc_t fat16;
    fat16.recognize = fat16_recognize;
    return fat16;
}

void _fat16_read(vfs_device_t *t_vfs_dev, uint32_t t_addr, uint8_t *t_buf) {
    printf("DRIVER ID:"); printd(t_vfs_dev->dev.driver_id);
    void (*rd)(device_t *d, uint32_t s, uint8_t *r) = drivers[t_vfs_dev->dev.driver_id].driver_desc.functions[DRIVER_STORAGE_READ];
    rd(&t_vfs_dev->dev, t_addr, t_buf);
}

void _fat16_write(vfs_device_t *t_vfs_dev, uint32_t t_addr, uint8_t *t_buf, uint32_t t_buf_size) {
    void (*wr)(device_t *d, uint32_t s, uint8_t *r, uint32_t bs) = drivers[t_vfs_dev->dev.driver_id].driver_desc.functions[DRIVER_STORAGE_WRITE];
    wr(&t_vfs_dev->dev, t_addr, t_buf, t_buf_size);
}

// Public

void fat16_install() {
    vfs_add_fs(_fat16_fs_desc());
}

bool fat16_recognize(vfs_device_t *t_vfs_dev) {
    uint8_t res[512];
    _fat16_read(t_vfs_dev, 0, res);
    char fat16Signature[] = {'F', 'A', 'T', '1', '6', 0x20, 0x20, 0x20};
    bool isSignatureCorrect = true;
    for (uint16_t i = 0x36; i < 0x36 + 8; i++) {
        isSignatureCorrect &= (fat16Signature[i - 0x36] == res[i]);
    }
    return isSignatureCorrect;
}
