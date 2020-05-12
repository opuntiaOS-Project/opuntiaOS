#include <drivers/driver_manager.h>
#include <cmd/cmd.h>

// ------------
// Private
// ------------

static uint8_t _drivers_count = 0;
static uint8_t _devices_count = 0;

driver_t drivers[MAX_DRIVERS_COUNT];
device_t devices[MAX_DEVICES_COUNT];

int16_t _driver_for_device(device_desc_t t_device_info);
void _no_driver_for_device(device_desc_t t_device_info);

// Private
// Handler if device hasn't driver
void _no_driver_for_device(device_desc_t t_device_info) {
    kprintf("No driver for the device\n\n");
}

// Private
// Try to find the best capable driver for a device.
int16_t _driver_for_device(device_desc_t t_device_info) {
    uint8_t cur_capablity = 0;
    uint8_t best_capablity = 0;
    uint8_t best_capable_driver_id = 0;
    for (uint8_t i = 0; i < _drivers_count; i++) {
        if (!drivers[i].driver_desc.is_device_driver) {
            continue;
        }
        cur_capablity = 0;
        if (drivers[i].driver_desc.pci_serve_class == t_device_info.class_id) {
            cur_capablity |= 0b1000;
        }
        if (drivers[i].driver_desc.pci_serve_subclass == t_device_info.subclass_id) {
            cur_capablity |= 0b0100;
        }
        if (drivers[i].driver_desc.pci_serve_vendor_id == t_device_info.vendor_id) {
            cur_capablity |= 0b0010;
        }
        if (drivers[i].driver_desc.pci_serve_device_id == t_device_info.device_id) {
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

// Debug
// Print all drivers
char _debug_drivers_bool_to_char(bool b) {
    if (b) {
        return 'T';
    }
    return 'F';
}

char _debug_drivers_print_name(int d_id) {
    char *tmp = kmalloc(DRIVER_NAME_LENGTH + 1);
    for (int i = 0; i < DRIVER_NAME_LENGTH; i++) {
        tmp[i] = drivers[d_id].driver_desc.name[i];
    }
    tmp[DRIVER_NAME_LENGTH] = '\0';
    kprintf(tmp);
    kfree(tmp);
}

void _debug_drivers_print_status(int d_id) {
    char *tmp = " \0";
    tmp[0] = _debug_drivers_bool_to_char(drivers[d_id].is_active);
    kprintf(tmp);
}

void _debug_drivers_print_start_params(int d_id) {
    char *tmp = " \0";
    tmp[0] = _debug_drivers_bool_to_char(drivers[d_id].driver_desc.auto_start);
    kprintf(tmp);
    tmp[0] = _debug_drivers_bool_to_char(drivers[d_id].driver_desc.is_device_driver);
    kprintf(tmp);
    tmp[0] = _debug_drivers_bool_to_char(drivers[d_id].driver_desc.is_device_needed);
    kprintf(tmp);
    tmp[0] = _debug_drivers_bool_to_char(drivers[d_id].driver_desc.is_driver_needed);
    kprintf(tmp);
}

void _debug_drivers_list() {
    for (int i = 0; i < _drivers_count; i++) {
        kprintf("Driver: "); kprintd(i); kprintf("\n");
        kprintf("Id: "); kprintd(drivers[i].id); kprintf("\n");
        kprintf("Name: [currently unsupported]"); kprintf("\n");
        kprintf("Active: "); _debug_drivers_print_status(i); kprintf("\n");
        kprintf("Type: "); kprintd(drivers[i].driver_desc.type); kprintf("\n");
        kprintf("Params: "); _debug_drivers_print_start_params(i); kprintf("\n");
        if (drivers[i].driver_desc.is_device_driver) {
            kprintf("PCI_SERVE_CLASS: "); kprintd(drivers[i].driver_desc.pci_serve_class); kprintf("\n");
            kprintf("PCI_SERVE_SUBCLASS: "); kprintd(drivers[i].driver_desc.pci_serve_subclass); kprintf("\n");
            kprintf("PCI_SERVE_VENDOR: "); kprintd(drivers[i].driver_desc.pci_serve_vendor_id); kprintf("\n");
            kprintf("PCI_SERVE_DEVICE: "); kprintd(drivers[i].driver_desc.pci_serve_device_id); kprintf("\n");
        }
        kprintf("-------\n");
    }
}


// ------------
// Public
// ------------

// Init the Driver Manager
bool driver_manager_init() {
    cmd_register("drivers", _debug_drivers_list);
    return true;
}

// Registering new driver
void driver_install(driver_desc_t t_driver_info) {
    driver_t new_driver;
    new_driver.id = _drivers_count;
    new_driver.driver_desc = t_driver_info;
    drivers[_drivers_count++] = new_driver;
}

// Turn on all drivers which don't need devices
void drivers_run() {
    for (uint8_t i = 0; i < _drivers_count; i++) {
        drivers[i].is_active = false;
    }

    for (uint8_t i = 0; i < _drivers_count; i++) {
        if (drivers[i].driver_desc.auto_start) {
            // starting driver
            drivers[i].is_active = true;
            void (*rd)() = (void*)drivers[i].driver_desc.functions[0];
            rd();
        }
    }
    pass_drivers_to_master_drivers();
    pass_devices_to_master_drivers();
}

// pass_drivers_to_master_drivers passes drivers to a master driver
void pass_drivers_to_master_drivers() {
    for (uint8_t i = 0; i < _drivers_count; i++) {
        if (drivers[i].driver_desc.is_driver_needed) {
            for (uint8_t j = 0; j < _drivers_count; j++) {
                if (drivers[i].driver_desc.type_of_needed_driver == drivers[j].driver_desc.type) {
                    drivers[i].is_active = true;
                    drivers[j].is_active = true;
                    void (*rd)(driver_t *nd) = (void*)drivers[i].driver_desc.functions[0];
                    rd(&drivers[j]);
                }
            }
        }
    }
}

// pass_devices_to_master_drivers passes devices with device_driver 
// to a master driver
void pass_devices_to_master_drivers() {
    for (uint8_t i = 0; i < _drivers_count; i++) {
        if (drivers[i].driver_desc.is_device_needed) {
            for (uint8_t j = 0; j < _devices_count; j++) {
                if (drivers[i].driver_desc.type_of_needed_device == devices[j].type) {
                    drivers[i].is_active = true;
                    void (*rd)(device_t *nd) = (void*)drivers[i].driver_desc.functions[1];
                    rd(&devices[j]);
                }
            }
        }
    }
}

// device_install registers a new device and find a driver for the device
void device_install(device_desc_t t_device_info) {
    uint8_t dev_id = _devices_count++;
    devices[dev_id].id = dev_id;
    devices[dev_id].driver_id = _driver_for_device(t_device_info);
    devices[dev_id].device_desc = t_device_info;
    
    if (devices[dev_id].driver_id == -1) {
        devices[dev_id].type = DEVICE_BAD_SIGN;
        _no_driver_for_device(t_device_info);
    } else {
        devices[dev_id].type = drivers[devices[dev_id].driver_id].driver_desc.type;
        void (*rd)(device_t *nd) = (void*)drivers[devices[dev_id].driver_id].driver_desc.functions[0];
        rd(&devices[dev_id]);
    }
}

// Should be called when a device was ejected

void _ask_driver_to_eject_device(uint8_t driver_id, uint8_t dev_id) {
    if (drivers[driver_id].driver_desc.type == DRIVER_VIRTUAL_FILE_SYSTEM) {
        void (*ej)(device_t *nd) = (void*)drivers[driver_id].driver_desc.functions[DRIVER_VIRTUAL_FILE_SYSTEM_EJECT_DEVICE];
        ej(&devices[dev_id]);
    }
}

void eject_device(uint8_t dev_id) {
    uint8_t used_driver_id = devices[dev_id].driver_id;
    _ask_driver_to_eject_device(used_driver_id, dev_id);

    for (uint8_t i = 0; i < _drivers_count; i++) {
        if (drivers[i].driver_desc.is_device_needed) {
            if (drivers[i].driver_desc.type_of_needed_device == devices[dev_id].type) {
                _ask_driver_to_eject_device(i, dev_id);
            }
        }
    }
}

void eject_all_devices() {
    for (uint8_t dev_id = 0; dev_id < _devices_count; dev_id++) {
        eject_device(dev_id);
    }
}

// Get first device of Type staring with StartPos
device_t get_device(uint8_t t_dev_type, uint8_t t_start) {
    for (uint8_t i = t_start; i < _devices_count; i++) {
        if (devices[i].type == t_dev_type) {
            return devices[i];
        }
    }
    device_t bad_device;
    bad_device.type = DEVICE_BAD_SIGN;
    return bad_device;
}