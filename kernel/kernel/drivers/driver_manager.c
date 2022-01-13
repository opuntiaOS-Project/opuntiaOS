#include <drivers/driver_manager.h>
#include <libkern/libkern.h>
#include <libkern/log.h>

// #define DEBUG_DM

static int _drivers_count = 0;
static int _devices_count = 0;

driver_t drivers[MAX_DRIVERS_COUNT];
device_t devices[MAX_DEVICES_COUNT];

int devman_init()
{
    return 0;
}

static size_t devices_count()
{
    return _devices_count;
}

static size_t next_device_id()
{
    int res = devices_count();
    _devices_count++;
    return res;
}

static size_t drivers_count()
{
    return _drivers_count;
}

static size_t next_driver_id()
{
    int res = drivers_count();
    _drivers_count++;
    return res;
}

static int notify_drivers_about_device(device_t* new_device)
{
    for (int i = 0; i < drivers_count(); i++) {
        // Sending notification to existing drivers about the new device.
        if (TEST_FLAG(drivers[i].desc.listened_device_mask, new_device->type)) {
            if (drivers[i].desc.system_funcs.recieve_notification) {
                drivers[i].desc.system_funcs.recieve_notification(DEVMAN_NOTIFICATION_NEW_DEVICE, (uint32_t)new_device);
            }
        }
    }

    return 0;
}

static int notify_drivers_about_driver(driver_t* new_driver)
{
    for (int i = 0; i < drivers_count(); i++) {
        // Sending notification to existing drivers about the new one.
        if (TEST_FLAG(drivers[i].desc.listened_driver_mask, new_driver->desc.type)) {
            if (drivers[i].desc.system_funcs.recieve_notification) {
                drivers[i].desc.system_funcs.recieve_notification(DEVMAN_NOTIFICATION_NEW_DRIVER, (uint32_t)new_driver);
            }
        }

        // Letting the new driver know about the already created ones.
        if (TEST_FLAG(new_driver->desc.listened_driver_mask, drivers[i].desc.type)) {
            if (drivers[i].desc.system_funcs.recieve_notification) {
                drivers[i].desc.system_funcs.recieve_notification(DEVMAN_NOTIFICATION_NEW_DRIVER, (uint32_t)&drivers[i]);
            }
        }
    }

    return 0;
}

static bool does_driver_support_device(device_t* device, driver_t* driver)
{
    if (device->device_desc.type == DEVICE_DESC_PCI) {
        if (TEST_FLAG(driver->desc.listened_device_mask, device->type)) {
            int err = driver->desc.system_funcs.init_with_dev(device);
            if (!err) {
                device->driver_id = driver->id;
                notify_drivers_about_device(device);
                return true;
            }
        }
    } else if (device->device_desc.type == DEVICE_DESC_DEVTREE) {
        if (strcmp(devtree_name_of_entry(device->device_desc.devtree.entry), driver->name) == 0) {
            driver->desc.system_funcs.init_with_dev(device);
            device->driver_id = driver->id;
            notify_drivers_about_device(device);
            return true;
        }
    }

    return false;
}

static int push_devices_to_driver(driver_t* target_driver)
{
    for (int i = 0; i < devices_count(); i++) {
        device_t* dev = &devices[i];
        if (dev->driver_id != DRIVER_ID_EMPTY) {
            continue;
        }

        if (does_driver_support_device(dev, target_driver)) {
            dev->driver_id = target_driver->id;
            notify_drivers_about_device(dev);
        }
    }

    return 0;
}

static int push_device_to_drivers(device_t* new_device)
{
    for (int i = 0; i < drivers_count(); i++) {
        if (does_driver_support_device(new_device, &drivers[i])) {
            new_device->driver_id = drivers[i].id;
            notify_drivers_about_device(new_device);
            return 0;
        }
    }

    new_device->driver_id = DRIVER_ID_EMPTY;
    return -1;
}

int devman_register_driver(driver_desc_t driver_info, const char* name)
{
    size_t driver_id = next_driver_id();
#ifdef DEBUG_DM
    log("DM: add driver %s - %d", name, driver_id);
#endif
    drivers[driver_id].id = driver_id;
    drivers[driver_id].desc = driver_info;
    drivers[driver_id].name = name;

    push_devices_to_driver(&drivers[driver_id]);
    notify_drivers_about_driver(&drivers[driver_id]);
    return 0;
}

// device_install registers a new device and find a driver for the device
int devman_register_device(device_desc_t device_info, int type)
{
#ifdef DEBUG_DM
    log("DM: add device %d", type);
#endif
    size_t device_id = next_device_id();
    devices[device_id].id = device_id;
    devices[device_id].type = type;
    devices[device_id].device_desc = device_info;
    devices[device_id].is_virtual = false;

    int err = push_device_to_drivers(&devices[device_id]);
    return 0;
}

device_t* new_virtual_device(int type)
{
    size_t device_id = next_device_id();
    devices[device_id].id = device_id;
    devices[device_id].type = type;
    devices[device_id].is_virtual = true;
    return &devices[device_id];
}

// Turn on all drivers which don't need devices
void devman_run()
{
    for (int i = 0; i < _drivers_count; i++) {
        if (TEST_FLAG(drivers[i].desc.flags, DRIVER_DESC_FLAG_START)) {
            if (drivers[i].desc.system_funcs.on_start) {
#ifdef DEBUG_DM
                log("DM: run %s", drivers[i].name);
#endif
                drivers[i].desc.system_funcs.on_start();
            }
        }
    }
}

void devman_send_notification(uint32_t msg, uint32_t param)
{
    for (int i = 0; i < drivers_count(); i++) {
        if (drivers[i].desc.system_funcs.recieve_notification) {
            drivers[i].desc.system_funcs.recieve_notification(msg, param);
        }
    }
}

int devman_get_driver_id_by_name(const char* name)
{
    for (int i = 0; i < drivers_count(); i++) {
        if (strcmp(name, drivers[i].name) == 0) {
            return drivers[i].id;
        }
    }
    return -1;
}