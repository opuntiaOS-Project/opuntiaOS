#ifndef __oneOS__DRIVERS__DRIVERMANAGER_H
#define __oneOS__DRIVERS__DRIVERMANAGER_H

#include <types.h>
#include <x86/pci.h>

#define MAX_DRIVERS 256
#define MAX_DEVICES 64
#define MAX_DRIVER_FUNCTION_COUNT 5

enum DRIVERS_TYPE {
    DRIVER_STORAGE,
    DRIVER_VIDEO,
    DRIVER_SOUND,
    DRIVER_INPUT_SYSTEMS,
    DRIVER_NETWORK,
    DRIVER_CONTROLLER,
    DRIVER_BAD_SIGN = 0xff
};

enum DEVICES_TYPE {
    DEVICE_STORAGE,
    DEVICE_VIDEO,
    DEVICE_SOUND,
    DEVICE_INPUT_SYSTEMS,
    DEVICE_NETWORK,
    DEVICE_BAD_SIGN = 0xff
};

enum DRIVER_STORAGE_OPERTAION {
    DRIVER_STORAGE_ADD_DEVICE,
    DRIVER_STORAGE_READ,
    DRIVER_STORAGE_WRITE,
    DRIVER_STORAGE_FLUSH
};

enum DRIVER_INPUT_SYSTEMS_OPERTAION {
    DRIVER_INPUT_SYSTEMS_ADD_DEVICE,
    DRIVER_INPUT_SYSTEMS_GET_LAST_KEY,
    DRIVER_INPUT_SYSTEMS_DISCARD_LAST_KEY
};

typedef struct {
    uint8_t type;
    uint8_t pci_serve_class;
    uint8_t pci_serve_subclass;
    uint16_t pci_serve_vendor_id;
    uint16_t pci_serve_device_id;
    uint32_t functions[MAX_DRIVER_FUNCTION_COUNT];
} driver_desc_t;

typedef struct {
    uint8_t id;
    driver_desc_t driver_desc;
} driver_t;

typedef struct {
    uint8_t id;
    uint8_t type;
    int16_t driver_id;
    pcidd_t device_desc;
} device_t;

driver_t drivers[MAX_DRIVERS];
device_t devices[MAX_DEVICES];

void register_drivers();
void driver_install(driver_desc_t t_driver_info);
void device_install(pcidd_t t_device_info);
void print_drivers_list();
device_t get_device(uint8_t t_dev_type, uint8_t t_start);

#endif
