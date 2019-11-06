#include <drivers/driver_manager.h>

// ------------
// Private
// ------------

uint8_t _drivers_count = 0;
uint8_t _devices_count = 0;
int16_t _driver_for_device(device_desc_t t_device_info);
void _no_driver_for_device(device_desc_t t_device_info);

// Private
// Handler if device hasn't driver
void _no_driver_for_device(device_desc_t t_device_info) {
    printf("No driver for the device\n\n");
}

// Private
// Try to find the best capable driver for device.
int16_t _driver_for_device(device_desc_t t_device_info) {
    uint8_t cur_capablity = 0;
    uint8_t best_capablity = 0;
    uint8_t best_capable_driver_id = 0;
    for (uint8_t i = 0; i < _drivers_count; i++) {
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

// ------------
// Public
// ------------

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
        if (!drivers[i].driver_desc.need_device) {
            // starting driver
            void (*rd)() = drivers[i].driver_desc.functions[0];
            rd();
        }
    }
}

// Registering new device
void device_install(device_desc_t t_device_info) {
    device_t new_device;
    new_device.id = _devices_count;
    new_device.driver_id = _driver_for_device(t_device_info);
    new_device.device_desc = t_device_info;

    if (new_device.driver_id == -1) {
        new_device.type = DEVICE_BAD_SIGN;
        _no_driver_for_device(t_device_info);
    } else {
        new_device.type = drivers[new_device.driver_id].driver_desc.type;
        void (*rd)(device_t nd) = drivers[new_device.driver_id].driver_desc.functions[0];
        rd(new_device);
    }

    devices[_devices_count++] = new_device;
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

// Debug
// Print all drivers
void print_drivers_list() {
    for (int i = 0; i < _drivers_count; i++) {
        printf("-------\n");
        printd(drivers[i].driver_desc.type); printf("\n");
        printd(drivers[i].driver_desc.pci_serve_class); printf("\n");
        printd(drivers[i].driver_desc.pci_serve_subclass); printf("\n");
        printd(drivers[i].driver_desc.pci_serve_vendor_id); printf("\n");
        printd(drivers[i].driver_desc.pci_serve_device_id); printf("\n");
        printf("-------\n");
    }
}
