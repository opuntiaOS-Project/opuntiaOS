#ifndef __oneOS__DRIVERS__DRIVERMANAGER_H
#define __oneOS__DRIVERS__DRIVERMANAGER_H

#include <types.h>

#define MAX_DRIVERS_COUNT 256
#define MAX_DEVICES_COUNT 64
#define MAX_DRIVER_FUNCTION_COUNT 8
#define DRIVER_NAME_LENGTH 8

// Supported driver's types
enum DRIVERS_TYPE {
    DRIVER_STORAGE_DEVICE,
    DRIVER_VIDEO_DEVICE,
    DRIVER_SOUND_DEVICE,
    DRIVER_INPUT_SYSTEMS_DEVICE,
    DRIVER_NETWORK_DEVICE,

    DRIVER_BUS_CONTROLLER,
    DRIVER_VIRTUAL_FILE_SYSTEM,
    DRIVER_FILE_SYSTEM,
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

// Api function of DRIVER_VIRTUAL_FILE_SYSTEM type
enum DRIVER_VIRTUAL_FILE_SYSTEM_OPERTAION {
    DRIVER_VIRTUAL_FILE_SYSTEM_ADD_DRIVER,
    DRIVER_VIRTUAL_FILE_SYSTEM_ADD_DEVICE,
    DRIVER_VIRTUAL_FILE_SYSTEM_EJECT_DEVICE,
};

// Api function of DRIVER_FILE_SYSTEM type
enum DRIVER_FILE_SYSTEM_OPERTAION {
    DRIVER_FILE_SYSTEM_RECOGNIZE,
    DRIVER_FILE_SYSTEM_CREATE_DIR,
    DRIVER_FILE_SYSTEM_LOOKUP_DIR,
    DRIVER_FILE_SYSTEM_REMOVE_DIR,

    DRIVER_FILE_SYSTEM_WRITE_FILE,
    DRIVER_FILE_SYSTEM_READ_FILE,
    DRIVER_FILE_SYSTEM_REMOVE_FILE,
    DRIVER_FILE_SYSTEM_EJECT_DEVICE,
};

typedef struct {
    uint8_t type;
    char name[DRIVER_NAME_LENGTH];
    bool auto_start; // True if need to assign to a device
    bool is_device_driver; // True if it's a dev driver (like ata driver or a mouse driver)
    bool is_device_needed; // True if need to assign to a device
    bool is_driver_needed; // True if need to assign to a device
    uint8_t type_of_needed_device;
    uint8_t type_of_needed_driver;
    uint8_t pci_serve_class;
    uint8_t pci_serve_subclass;
    uint16_t pci_serve_vendor_id;
    uint16_t pci_serve_device_id;
    void* functions[MAX_DRIVER_FUNCTION_COUNT];
} driver_desc_t; // driver decriptor

typedef struct {
    uint8_t id;
    bool is_active;
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
    uint8_t type;
    int16_t driver_id;
    device_desc_t device_desc;
} device_t; // device

extern driver_t drivers[MAX_DRIVERS_COUNT];
extern device_t devices[MAX_DEVICES_COUNT];

bool driver_manager_init();
void register_drivers();
void driver_install(driver_desc_t t_driver_info);
void device_install(device_desc_t t_device_info);
void eject_device(uint8_t dev_id);
void eject_all_devices();
void drivers_run();
device_t get_device(uint8_t t_dev_type, uint8_t t_start);
void pass_drivers_to_master_drivers();
void pass_devices_to_master_drivers();

#endif // __oneOS__DRIVERS__DRIVERMANAGER_H
