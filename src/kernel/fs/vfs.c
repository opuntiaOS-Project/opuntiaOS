#include <fs/vfs.h>
#include <drivers/display.h>
#include <utils/mem.h>
#include <mem/malloc.h>

// Private
static vfs_device_t _vfs_devices[MAX_DEVICES_COUNT];
static fs_desc_t _vfs_fses[MAX_DRIVERS_COUNT];
static uint8_t _vfs_fses_count; // Will be deleted in next builds
static char next_disk_name;

uint8_t _vfs_get_drive_id(const char* path);
int8_t _vfs_get_dot_pos_in_filename(const char* t_filename);
int8_t _vfs_split_filename(char* t_filename);

// Private implementation

uint8_t _vfs_get_drive_id(const char* path) {
    char search_for = 'C';
    for (int i = 0; i < MAX_DEVICES_COUNT; i++) {
        if (_vfs_devices[i].disk_name == search_for) {
            return i;
        }
    }
    return 0;
}

int8_t _vfs_get_dot_pos_in_filename(const char* t_filename) {
    int8_t i = 0;
    int8_t dot_lst = -1;
    while (t_filename[i] != '\0') {
        if (t_filename[i] == '.') {
            dot_lst = i;
        }
        i++;
    }
    return dot_lst;
}

int8_t _vfs_split_filename(char* t_filename) {
    int8_t i = 0;
    int8_t dot_lst = _vfs_get_dot_pos_in_filename(t_filename);
    if (dot_lst == -1) {
        return -1;
    }
    t_filename[dot_lst] = '\0';
    return dot_lst + 1;
}

// Public implementation

driver_desc_t _vfs_driver_info();

driver_desc_t _vfs_driver_info() {
    driver_desc_t vfs_desc;
    vfs_desc.type = DRIVER_VIRTUAL_FILE_SYSTEM;
    vfs_desc.is_device_driver = false;
    vfs_desc.is_device_needed = true;
    vfs_desc.is_driver_needed = true;
    vfs_desc.type_of_needed_device = DEVICE_STORAGE;
    vfs_desc.type_of_needed_driver = DRIVER_FILE_SYSTEM;
    vfs_desc.functions[DRIVER_VIRTUAL_FILE_SYSTEM_ADD_DEVICE] = vfs_add_device;
    vfs_desc.functions[DRIVER_VIRTUAL_FILE_SYSTEM_ADD_DRIVER] = vfs_add_fs;
    vfs_desc.functions[DRIVER_VIRTUAL_FILE_SYSTEM_EJECT_DEVICE] = vfs_eject_device;
    return vfs_desc;
}

void vfs_install() {
    next_disk_name = 'C';
    driver_install(_vfs_driver_info());
}

// void vfs_find_devices() {
//     device_t cur_dev;
//     uint8_t start_s = 0;
//     cur_dev.type = DEVICE_STORAGE;
//     while (cur_dev.type != DEVICE_BAD_SIGN) {
//         cur_dev = get_device(cur_dev.type, start_s);
//         if (cur_dev.type != DEVICE_BAD_SIGN) {
//             vfs_add_device(&cur_dev);
//         }
//         start_s = cur_dev.id + 1;
//     }
// }

void vfs_add_device(device_t *dev) {
    if (dev->type != DEVICE_STORAGE) {
        return;
    }
    _vfs_devices[dev->id].dev = *dev;
    for (uint8_t i = 0; i < _vfs_fses_count; i++) {
        bool (*is_capable)(vfs_device_t *nd) = _vfs_fses[i].recognize;
        if (is_capable(&_vfs_devices[dev->id])) {
            _vfs_devices[dev->id].disk_name = next_disk_name++;
            _vfs_devices[dev->id].fs = i;
            return;
        }
    }
    printf("Can't find FS\n");
}

// TODO: reuse unused slots
void vfs_eject_device(device_t *dev) {
    printf("Ejecting\n");
    uint8_t fs_id = _vfs_devices[dev->id].fs;
    bool (*eject)(vfs_device_t *nd) = _vfs_fses[fs_id].eject_device;
    eject(&_vfs_devices[dev->id]);
}

void vfs_add_fs(driver_t *t_new_driver) {
    if (t_new_driver->driver_desc.type != DRIVER_FILE_SYSTEM) {
        return;
    }
    fs_desc_t new_fs;
    new_fs.recognize = t_new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_RECOGNIZE];
    new_fs.create_dir = t_new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_CREATE_DIR];
    new_fs.lookup_dir = t_new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_LOOKUP_DIR];
    new_fs.remove_dir = t_new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_REMOVE_DIR];
    new_fs.write_file = t_new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_WRITE_FILE];
    new_fs.read_file = t_new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_READ_FILE];
    new_fs.remove_file = t_new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_REMOVE_FILE];
    new_fs.eject_device = t_new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_EJECT_DEVICE];
    _vfs_fses[_vfs_fses_count++] = new_fs;
}

uint32_t vfs_lookup_dir(const char *t_path, vfs_element_t *t_buf) {
    uint8_t drive_id = _vfs_get_drive_id(t_path);
    uint32_t (*func)(vfs_device_t*, const char *, vfs_element_t*) = _vfs_fses[_vfs_devices[drive_id].fs].lookup_dir;
    return func(&_vfs_devices[drive_id], t_path, t_buf);
}

bool vfs_create_dir(const char* t_path, const char* t_dir_name) {
    uint8_t drive_id = _vfs_get_drive_id(t_path);
    bool (*func)(vfs_device_t*, const char*, const char*) = _vfs_fses[_vfs_devices[drive_id].fs].create_dir;
    return func(&_vfs_devices[drive_id], t_path, t_dir_name);
}

bool vfs_remove_dir(const char* path, const char* dir_name) {
    uint8_t drive_id = _vfs_get_drive_id(path);
    bool (*func)(vfs_device_t*, const char*, const char*) = _vfs_fses[_vfs_devices[drive_id].fs].remove_dir;
    return func(&_vfs_devices[drive_id], path, dir_name);
}

void vfs_write_file(const char *t_path, const char *t_file_name, const uint8_t *t_data, uint32_t t_size) {
    uint8_t size = 0;
    while (t_file_name[size] != '\0') size++;

    char *filename = kmalloc(size+1);
    memcpy(filename, t_file_name, size+1);

    int8_t ext_offset = _vfs_split_filename(filename);

    uint8_t drive_id = _vfs_get_drive_id(t_path);
    bool (*func)(vfs_device_t *t_vfs_dev, const char *t_path, const char *t_file_name, const char *t_file_ext, const uint8_t *t_data, uint32_t t_size) = _vfs_fses[_vfs_devices[drive_id].fs].write_file;
    func(&_vfs_devices[drive_id], t_path, filename, filename+ext_offset, t_data, t_size);
    kfree(filename);
}

void* vfs_read_file(const char *t_path, const char *t_file_name, uint16_t t_offset, int16_t t_len) {
    uint8_t size = 0;
    while (t_file_name[size] != '\0') size++;

    char *filename = kmalloc(size+1);
    memcpy(filename, t_file_name, size+1);

    int8_t ext_offset = _vfs_split_filename(filename);
    if (ext_offset == -1) {
        return 0;
    }

    uint8_t drive_id = _vfs_get_drive_id(t_path);
    void* (*func)(vfs_device_t *t_vfs_dev, const char *t_path, const char *t_file_name, const char *t_file_ext, uint16_t t_offset, int16_t t_len) = _vfs_fses[_vfs_devices[drive_id].fs].read_file;
    char *tmp = func(&_vfs_devices[drive_id], t_path, filename, filename+ext_offset, t_offset, t_len);
    kfree(filename);
    return tmp;
}

bool vfs_remove_file(const char *t_path, const char *t_file_name) {
    uint8_t size = 0;
    while (t_file_name[size] != '\0') size++;

    char *filename = kmalloc(size+1);
    memcpy(filename, t_file_name, size+1);

    int8_t ext_offset = _vfs_split_filename(filename);
    if (ext_offset == -1) {
        return 0;
    }

    uint8_t drive_id = _vfs_get_drive_id(t_path);
    void* (*func)(vfs_device_t *vfs_dev, const char *path, const char *filename, const char *file_ext) = _vfs_fses[_vfs_devices[drive_id].fs].remove_file;
    char *tmp = func(&_vfs_devices[drive_id], t_path, filename, filename+ext_offset);
    kfree(filename);
    // TODO check that
    return true;
}

vfs_element_t vfs_get_file_info(const char *t_path, const char *t_file_name) {
    vfs_element_t no_elem;
    no_elem.attributes = VFS_ATTR_NOTFILE;
    uint8_t size = 0;
    while (t_file_name[size] != '\0') size++;

    char *filename = kmalloc(size+1);
    memcpy(filename, t_file_name, size+1);

    int8_t ext_offset = _vfs_split_filename(filename);
    if (ext_offset == -1) {
        return no_elem;
    }

    vfs_element_t vfs_buf[16]; // TODO 16 tmp value (will be replaced with std value)
    uint32_t n = vfs_lookup_dir(t_path, vfs_buf);
    for (int i = 0; i < n; i++) {
        uint32_t fname_len = strlen(vfs_buf[i].filename);
        uint32_t fext_len = strlen(vfs_buf[i].filename_ext);

        if (fname_len != ext_offset - 1) {
            continue;
        }
        if (fext_len != (size - ext_offset)) {
            continue;
        }
        int cmp1 = memcmp(filename, vfs_buf[i].filename, fname_len);
        int cmp2 = memcmp(filename+ext_offset, vfs_buf[i].filename_ext, fext_len);
        if (cmp1 == 0 && cmp2 == 0) {
            return vfs_buf[i];
        }
    }
    return no_elem;
}

void vfs_test() {
    // vfs_lookup_dir(0);
    // vfs_create_dir("/", "hello");
    // vfs_write_file("/", "hello", "exe", "kek", 3);
    // printf((uint8_t *)vfs_read_file("/", "hello", "exe", 0, -1));
    // vfs_lookup_dir("/");
    // vfs_lookup_dir("/a/");
}
