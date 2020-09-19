#include <drivers/driver_manager.h>
#include <log.h>

// ------------
// Private
// ------------

static int _drivers_count = 0;
static int _devices_count = 0;

driver_t drivers[MAX_DRIVERS_COUNT];
device_t devices[MAX_DEVICES_COUNT];

int16_t _driver_for_device(device_desc_t device_info);
void _no_driver_for_device(device_desc_t device_info);

static void _dm_no_driver_for_device(device_desc_t device_info)
{
    log_warn("No driver for the device: cl/subcl %x %x : ven/dev %x %x", device_info.class_id, device_info.subclass_id, device_info.vendor_id, device_info.device_id);
}

static int _dm_find_driver_for_device(device_desc_t device_info)
{
    uint8_t cur_capablity = 0;
    uint8_t best_capablity = 0;
    uint8_t best_capable_driver_id = 0;
    for (int i = 0; i < _drivers_count; i++) {
        if (!drivers[i].desc.is_device_driver) {
            continue;
        }
        cur_capablity = 0;
        if (drivers[i].desc.pci_serve_class == device_info.class_id) {
            cur_capablity |= 0b1000;
        }
        if (drivers[i].desc.pci_serve_subclass == device_info.subclass_id) {
            cur_capablity |= 0b0100;
        }
        if (drivers[i].desc.pci_serve_vendor_id == device_info.vendor_id) {
            cur_capablity |= 0b0010;
        }
        if (drivers[i].desc.pci_serve_device_id == device_info.device_id) {
            cur_capablity |= 0b0001;
        }
        if (cur_capablity > best_capablity) {
            best_capablity = cur_capablity;
            best_capable_driver_id = i;
        }
    }
    if (best_capablity >= 0b1100) {
        return best_capable_driver_id;
    }
    return -1;
}

// Init the Driver Manager
bool driver_manager_init()
{
    return true;
}

// Registering new driver
void driver_install(driver_desc_t driver_info)
{
    driver_t new_driver;
    new_driver.id = _drivers_count;
    new_driver.desc = driver_info;
    drivers[_drivers_count++] = new_driver;
}

// Turn on all drivers which don't need devices
void drivers_run()
{
    for (int i = 0; i < _drivers_count; i++) {
        drivers[i].is_active = false;
    }

    for (int i = 0; i < _drivers_count; i++) {
        if (drivers[i].desc.auto_start) {
            drivers[i].is_active = true;
            void (*rd)() = (void*)drivers[i].desc.functions[DM_FUNC_DRIVER_START];
            rd();
        }
    }
    pass_drivers_to_master_drivers();
    pass_devices_to_master_drivers();
}

void pass_drivers_to_master_drivers()
{
    for (int i = 0; i < _drivers_count; i++) {
        if (drivers[i].desc.is_driver_needed) {
            for (int j = 0; j < _drivers_count; j++) {
                if (drivers[i].desc.type_of_needed_driver == drivers[j].desc.type) {
                    drivers[i].is_active = true;
                    drivers[j].is_active = true;
                    void (*rd)(driver_t* nd) = (void*)drivers[i].desc.functions[DM_FUNC_DRIVER_EMIT_DRIVER];
                    rd(&drivers[j]);
                }
            }
        }
    }
}

void pass_devices_to_master_drivers()
{
    for (int i = 0; i < _drivers_count; i++) {
        if (drivers[i].desc.is_device_needed) {
            for (int j = 0; j < _devices_count; j++) {
                if (drivers[i].desc.type_of_needed_device == devices[j].type) {
                    drivers[i].is_active = true;
                    void (*rd)(device_t * nd) = (void*)drivers[i].desc.functions[DM_FUNC_DRIVER_EMIT_DEVICE];
                    rd(&devices[j]);
                }
            }
        }
    }
}

// device_install registers a new device and find a driver for the device
void device_install(device_desc_t device_info)
{
    int dev_id = _devices_count++;
    devices[dev_id].id = dev_id;
    devices[dev_id].driver_id = _dm_find_driver_for_device(device_info);
    devices[dev_id].device_desc = device_info;
    devices[dev_id].is_virtual = false;

    if (devices[dev_id].driver_id == -1) {
        devices[dev_id].type = DEVICE_BAD_SIGN;
        _dm_no_driver_for_device(device_info);
    } else {
        devices[dev_id].type = drivers[devices[dev_id].driver_id].desc.type;
        void (*rd)(device_t * nd) = (void*)drivers[devices[dev_id].driver_id].desc.functions[DM_FUNC_DEVICE_START];
        rd(&devices[dev_id]);
    }
}

// Should be called when a device was ejected

void _ask_driver_to_eject_device(uint8_t driver_id, uint8_t dev_id)
{
    if (drivers[driver_id].desc.type == DRIVER_VIRTUAL_FILE_SYSTEM) {
        void (*ej)(device_t * nd) = (void*)drivers[driver_id].desc.functions[DRIVER_VIRTUAL_FILE_SYSTEM_EJECT_DEVICE];
        ej(&devices[dev_id]);
    }
}

void eject_device(uint8_t dev_id)
{
    uint8_t used_driver_id = devices[dev_id].driver_id;
    _ask_driver_to_eject_device(used_driver_id, dev_id);

    for (int i = 0; i < _drivers_count; i++) {
        if (drivers[i].desc.is_device_needed) {
            if (drivers[i].desc.type_of_needed_device == devices[dev_id].type) {
                _ask_driver_to_eject_device(i, dev_id);
            }
        }
    }
}

void eject_all_devices()
{
    for (int dev_id = 0; dev_id < _devices_count; dev_id++) {
        eject_device(dev_id);
    }
}

// Get first device of Type staring with StartPos
device_t get_device(uint8_t dev_type, uint8_t start)
{
    for (int i = start; i < _devices_count; i++) {
        if (devices[i].type == dev_type) {
            return devices[i];
        }
    }
    device_t bad_device;
    bad_device.type = DEVICE_BAD_SIGN;
    return bad_device;
}

device_t* new_virtual_device(uint8_t type)
{
    int dev_id = _devices_count++;
    devices[dev_id].id = dev_id;
    devices[dev_id].type = type;
    devices[dev_id].is_virtual = true;
    return &devices[dev_id];
}

void dm_send_notification(uint32_t msg, uint32_t param)
{
    for (int i = 0; i < _drivers_count; i++) {
        if (drivers[i].desc.functions[DM_FUNC_NOTIFY]) {
            void (*notify)(uint32_t, uint32_t) = (void*)drivers[i].desc.functions[DM_FUNC_NOTIFY];
            notify(msg, param);
        }
    }
}