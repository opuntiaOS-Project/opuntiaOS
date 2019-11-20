#include <fs/vfs.h>

// Private
vfs_device_t _vfs_devices[VFS_MAX_DEV_COUNT];
fs_desc_t _vfs_fses[VFS_MAX_FS_COUNT];
_vfs_devices_count = 0;
_vfs_fses_count = 0;

uint8_t _vfs_get_drive_id(const char* path);

// Private implementation

uint8_t _vfs_get_drive_id(const char* path) {
    return 0;
}

// Public implementation

void vfs_install() {
    device_t cur_dev;
    uint8_t start_s = 0;
    cur_dev.type = DEVICE_STORAGE;
    while (cur_dev.type != DEVICE_BAD_SIGN) {
        cur_dev = get_device(cur_dev.type, start_s);
        if (cur_dev.type != DEVICE_BAD_SIGN) {
            vfs_add_device(cur_dev);
        }
        start_s = cur_dev.id + 1;
    }
}

void vfs_add_device(device_t t_new_dev) {
    if (t_new_dev.type != DEVICE_STORAGE) {
        return;
    }
    _vfs_devices[_vfs_devices_count].dev = t_new_dev;
    for (uint8_t i = 0; i < _vfs_fses_count; i++) {
        bool (*is_capable)(vfs_device_t *nd) = _vfs_fses[i].recognize;
        if (is_capable(&_vfs_devices[_vfs_devices_count])) {
            _vfs_devices[_vfs_devices_count++].fs = i;
            return;
        }
    }
    printf("Can't find FS\n");
}

void vfs_add_fs(fs_desc_t t_new_fs) {
    _vfs_fses[_vfs_fses_count++] = t_new_fs;
}

void vfs_lookup_dir(const char *t_path) {
    uint8_t drive_id = _vfs_get_drive_id(t_path);
    vfs_element_t vfs_buf[16]; // 16 tmp value (will be replaced with std value)
    uint32_t (*func)(vfs_device_t*, const char *, vfs_element_t*) = _vfs_fses[drive_id].lookup_dir;
    uint32_t n = func(&_vfs_devices[drive_id], t_path, vfs_buf);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 8; j++) {
            char *text = " \0";
            text[0] = vfs_buf[i].filename[j];
            printf(text);
        }
        printf("\n");
    }
}

bool vfs_create_dir(const char* t_path, const char* t_dir_name) {
    uint8_t drive_id = _vfs_get_drive_id(t_path);
    bool (*func)(vfs_device_t*, const char*, const char*) = _vfs_fses[drive_id].create_dir;
    return func(&_vfs_devices[drive_id], t_path, t_dir_name);
}

void vfs_write_file(const char *t_path, const char *t_file_name, const char *t_file_ext, const uint8_t *t_data, uint32_t t_size) {
    uint8_t drive_id = _vfs_get_drive_id(t_path);
    bool (*func)(vfs_device_t *t_vfs_dev, const char *t_path, const char *t_file_name, const char *t_file_ext, const uint8_t *t_data, uint32_t t_size) = _vfs_fses[drive_id].write_file;
    func(&_vfs_devices[drive_id], t_path, t_file_name, t_file_ext, t_data, t_size);
}

void* vfs_read_file(const char *t_path, const char *t_file_name, const char *t_file_ext, uint16_t t_offset, int16_t t_len) {
    uint8_t drive_id = _vfs_get_drive_id(t_path);
    void* (*func)(vfs_device_t *t_vfs_dev, const char *t_path, const char *t_file_name, const char *t_file_ext, uint16_t t_offset, int16_t t_len) = _vfs_fses[drive_id].read_file;
    return func(&_vfs_devices[drive_id], t_path, t_file_name, t_file_ext, t_offset, t_len);
}

void vfs_test() {
    // vfs_lookup_dir(0);
    // vfs_create_dir("/", "hello");
    // vfs_write_file("/", "hello", "exe", "kek", 3);
    // printf((uint8_t *)vfs_read_file("/", "hello", "exe", 0, -1));
    // vfs_lookup_dir("/");
    // vfs_lookup_dir("/a/");
}
