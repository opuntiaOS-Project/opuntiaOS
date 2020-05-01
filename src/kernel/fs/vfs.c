#include <drivers/display.h>
#include <fs/vfs.h>
#include <mem/malloc.h>
#include <utils/mem.h>

// Private
static vfs_device_t _vfs_devices[MAX_DEVICES_COUNT];
static fs_desc_t _vfs_fses[MAX_DRIVERS_COUNT];
static uint8_t _vfs_fses_count; // Will be deleted in next builds
static uint32_t root_fs_dev_id;

uint8_t _vfs_get_drive_id(const char* path);
int8_t _vfs_get_dot_pos_in_filename(const char* t_filename);
int8_t _vfs_split_filename(char* t_filename);

// Private implementation

// Public implementation

driver_desc_t _vfs_driver_info();

driver_desc_t _vfs_driver_info()
{
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

void vfs_install()
{
    driver_install(_vfs_driver_info());
}

void vfs_add_device(device_t* dev)
{
    if (dev->type != DEVICE_STORAGE) {
        return;
    }
    _vfs_devices[dev->id].dev = *dev;
    for (uint8_t i = 0; i < _vfs_fses_count; i++) {
        bool (*is_capable)(vfs_device_t * nd) = _vfs_fses[i].recognize;
        if (is_capable(&_vfs_devices[dev->id])) {
            // FIXME: currently last disk is a rootfs. May be to detect? :^)
            root_fs_dev_id = dev->id;
            _vfs_devices[dev->id].fs = i;
            return;
        }
    }
    printf("Can't find FS\n");
}

// TODO: reuse unused slots
void vfs_eject_device(device_t* dev)
{
    printf("Ejecting\n");
    uint8_t fs_id = _vfs_devices[dev->id].fs;
    bool (*eject)(vfs_device_t * nd) = _vfs_fses[fs_id].eject_device;
    eject(&_vfs_devices[dev->id]);
}

void vfs_add_fs(driver_t* t_new_driver)
{
    if (t_new_driver->driver_desc.type != DRIVER_FILE_SYSTEM) {
        return;
    }

    fs_desc_t new_fs;
    new_fs.open = t_new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_OPEN];
    new_fs.read = t_new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_READ];
    new_fs.write = t_new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_WRITE];
    new_fs.mkdir = t_new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_MKDIR];
    new_fs.recognize = t_new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_RECOGNIZE];
    new_fs.eject_device = t_new_driver->driver_desc.functions[DRIVER_FILE_SYSTEM_EJECT_DEVICE];
    _vfs_fses[_vfs_fses_count++] = new_fs;
}

int vfs_open(file_descriptor_t* base, const char* path, file_descriptor_t* fd)
{
    file_descriptor_t base_fd;
    if (!base) {
        // TODO: deprecate get_drive_id;
        base_fd.inode_index = 2;
        base_fd.dev_id = root_fs_dev_id;
        base = &base_fd;
    }
    uint32_t drive_id = base->dev_id;
    uint32_t (*func)(vfs_device_t*, file_descriptor_t*, const char*, file_descriptor_t*) = _vfs_fses[_vfs_devices[drive_id].fs].open;
    return func(&_vfs_devices[drive_id], base, path, fd);
}

int vfs_read(file_descriptor_t* fd, uint8_t* buf, uint32_t start, uint32_t len)
{
    uint32_t drive_id = fd->dev_id;
    uint32_t (*func)(vfs_device_t*, file_descriptor_t*, uint8_t*, uint32_t, uint32_t) = _vfs_fses[_vfs_devices[drive_id].fs].read;
    return func(&_vfs_devices[drive_id], fd, buf, start, len);
}

int vfs_write(file_descriptor_t* fd, uint8_t* buf, uint32_t start, uint32_t len)
{
    uint32_t drive_id = fd->dev_id;
    uint32_t (*func)(vfs_device_t*, file_descriptor_t*, uint8_t*, uint32_t, uint32_t) = _vfs_fses[_vfs_devices[drive_id].fs].write;
    return func(&_vfs_devices[drive_id], fd, buf, start, len);
}

int vfs_mkdir(file_descriptor_t* fd, const char* name)
{
    uint32_t drive_id = fd->dev_id;
    uint32_t (*func)(vfs_device_t*, file_descriptor_t*, const char*) = _vfs_fses[_vfs_devices[drive_id].fs].mkdir;
    return func(&_vfs_devices[drive_id], fd, name);
}

void vfs_test()
{
}
