#ifndef __oneOS__DRIVERS__DRIVERMANAGER_H
#define __oneOS__DRIVERS__DRIVERMANAGER_H

#include <types.h>

#define MAX_DRIVERS 256
#define MAX_DEVICES 64
#define MAX_DRIVER_FUNCTION_COUNT 5

// Supported driver's types
enum DRIVERS_TYPE {
    DRIVER_STORAGE,
    DRIVER_VIDEO,
    DRIVER_SOUND,
    DRIVER_INPUT_SYSTEMS,
    DRIVER_NETWORK,
    DRIVER_BUS_CONTROLLER,
    DRIVER_BAD_SIGN = 0xff
};

// Supported device's types
enum DEVICES_TYPE {
    DEVICE_STORAGE,
    DEVICE_VIDEO,
    DEVICE_SOUND,
    DEVICE_INPUT_SYSTEMS,
    DEVICE_NETWORK,
    DEVICE_BAD_SIGN = 0xff
};

// Api function of DRIVER_STORAGE type
enum DRIVER_STORAGE_OPERTAION {
    DRIVER_STORAGE_ADD_DEVICE, // function called when a device is found
    DRIVER_STORAGE_READ,
    DRIVER_STORAGE_WRITE,
    DRIVER_STORAGE_FLUSH
};

// Api function of DRIVER_INPUT_SYSTEMS type
enum DRIVER_INPUT_SYSTEMS_OPERTAION {
    DRIVER_INPUT_SYSTEMS_ADD_DEVICE, // function called when a device is found
    DRIVER_INPUT_SYSTEMS_GET_LAST_KEY,
    DRIVER_INPUT_SYSTEMS_DISCARD_LAST_KEY
};

// Api function of DRIVER_CONTROLLER type
enum DRIVER_BUS_CONTROLLER_OPERTAION {
    DRIVER_BUS_CONTROLLER_FIND_DEVICE // function called when a device is found
};

typedef struct {
    uint8_t type;
    bool need_device; // True if need to assign to a device
    uint8_t pci_serve_class;
    uint8_t pci_serve_subclass;
    uint16_t pci_serve_vendor_id;
    uint16_t pci_serve_device_id;
    uint32_t functions[MAX_DRIVER_FUNCTION_COUNT];
} driver_desc_t; // driver decriptor

typedef struct {
    uint8_t id;
    driver_desc_t driver_desc;
} driver_t; // driver

typedef struct {
    uint8_t bus;
    uint8_t device;
    uint8_t function;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t class_id;
    uint8_t subclass_id;
    uint8_t interface_id;
    uint8_t revision_id;
    uint32_t interrupt;
    uint32_t port_base;
} device_desc_t; // device desriptor

typedef struct {
    uint8_t id;
    uint8_t translate_id;
    uint8_t type;
    int16_t driver_id;
    device_desc_t device_desc;
} device_t; // device

driver_t drivers[MAX_DRIVERS];
device_t devices[MAX_DEVICES];

void register_drivers();
void driver_install(driver_desc_t t_driver_info);
void device_install(device_desc_t t_device_info);
void print_drivers_list();
device_t get_device(uint8_t t_dev_type, uint8_t t_start);

#endif // __oneOS__DRIVERS__DRIVERMANAGER_H
