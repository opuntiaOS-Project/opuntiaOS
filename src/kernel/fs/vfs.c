#include <fs/vfs.h>

// Private
vfs_device_t _vfs_devices[VFS_MAX_DEV_COUNT];
fs_desc_t _vfs_fses[VFS_MAX_FS_COUNT];
_vfs_devices_count = 0;
_vfs_fses_count = 0;


// Public

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

void vfs_lookup_dir(uint8_t t_drive_id, const char *path) {
    vfs_element_t vfs_buf[16]; // 16 tmp value (will be replaced with std value)
    uint32_t (*func)(vfs_device_t*, const char *, vfs_element_t*) = _vfs_fses[t_drive_id].lookup_dir;
    uint32_t n = func(&_vfs_devices[t_drive_id], path, vfs_buf);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 8; j++) {
            char *text = " \0";
            text[0] = vfs_buf[i].filename[j];
            printf(text);
        }
        printf("\n");
    }
}

void vfs_create_dir(uint8_t t_drive_id) {
    bool (*func)(vfs_device_t*, const char*, const char*) = _vfs_fses[t_drive_id].create_dir;
    func(&_vfs_devices[t_drive_id], "/", "a");
    func(&_vfs_devices[t_drive_id], "/", "b");
    // func(&_vfs_devices[t_drive_id], "/a/", "b");
}

void vfs_create_file(uint8_t t_drive_id) {
    bool (*func)(vfs_device_t *t_vfs_dev, const char *t_path, const char *t_file_name, const char *t_file_ext, const uint8_t *t_data, uint32_t t_size) = _vfs_fses[t_drive_id].write_file;
    func(&_vfs_devices[t_drive_id], "/", "file", "exe", "hello, world", 12);
}

void vfs_test() {
    // vfs_lookup_dir(0);
    vfs_create_dir(0);
    vfs_create_file(0);
    vfs_lookup_dir(0, "/");
    vfs_lookup_dir(0, "/a/");

}
