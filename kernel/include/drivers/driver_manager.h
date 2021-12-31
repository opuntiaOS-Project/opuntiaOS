/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_DRIVER_MANAGER_H
#define _KERNEL_DRIVERS_DRIVER_MANAGER_H

#include <libkern/types.h>

#define MINORBITS 20
#define MINORMASK ((1U << MINORBITS) - 1)

#define MAJOR(dev) ((unsigned int)((dev) >> MINORBITS))
#define MINOR(dev) ((unsigned int)((dev)&MINORMASK))
#define MKDEV(ma, mi) (((ma) << MINORBITS) | (mi))

#define MAX_DRIVERS_COUNT 256
#define MAX_DEVICES_COUNT 64
#define MAX_DRIVER_FUNCTION_COUNT 32
#define DRIVER_NAME_LENGTH 8

#define DM_FUNC_NOTIFY 0x0
#define DM_FUNC_DEVICE_START 0x1
#define DM_FUNC_DRIVER_START 0x1
#define DM_FUNC_DRIVER_EMIT_DRIVER 0x1
#define DM_FUNC_DRIVER_EMIT_DEVICE 0x2

enum DRIVER_MAN_NOTIFICATIONS {
    DM_NOTIFICATION_DEVFS_READY = 0,
    DM_NOTIFICATION_STOP,
};

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
    DRIVER_OTHER,
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

enum DRIVER_COMMON {
    DRIVER_NOTIFICATION = 0x0,
};

// Api function of DRIVER_STORAGE type
enum DRIVER_VIDEO_OPERTAION {
    DRIVER_VIDEO_INIT = 0x1, // function called when a device is found
    DRIVER_VIDEO_SET_RESOLUTION,
};

// Api function of DRIVER_STORAGE type
enum DRIVER_STORAGE_OPERTAION {
    DRIVER_STORAGE_ADD_DEVICE = 0x1, // function called when a device is found
    DRIVER_STORAGE_READ,
    DRIVER_STORAGE_WRITE,
    DRIVER_STORAGE_FLUSH,
    DRIVER_STORAGE_CAPACITY,
};

// Api function of DRIVER_INPUT_SYSTEMS type
enum DRIVER_INPUT_SYSTEMS_OPERTAION {
    DRIVER_INPUT_SYSTEMS_ADD_DEVICE = 0x1, // function called when a device is found
    DRIVER_INPUT_SYSTEMS_GET_LAST_KEY,
    DRIVER_INPUT_SYSTEMS_DISCARD_LAST_KEY
};

// Api function of DRIVER_CONTROLLER type
enum DRIVER_BUS_CONTROLLER_OPERTAION {
    DRIVER_BUS_CONTROLLER_FIND_DEVICE = 0x1, // function called when a device is found
};

// Api function of DRIVER_VIRTUAL_FILE_SYSTEM type
enum DRIVER_VIRTUAL_FILE_SYSTEM_OPERTAION {
    DRIVER_VIRTUAL_FILE_SYSTEM_ADD_DRIVER = 0x1,
    DRIVER_VIRTUAL_FILE_SYSTEM_ADD_DEVICE,
    DRIVER_VIRTUAL_FILE_SYSTEM_EJECT_DEVICE,
};

// Api function of DRIVER_FILE_SYSTEM type
enum DRIVER_FILE_SYSTEM_OPERTAION {
    DRIVER_FILE_SYSTEM_RECOGNIZE = 0x1,
    DRIVER_FILE_SYSTEM_PREPARE_FS,
    DRIVER_FILE_SYSTEM_EJECT_DEVICE,

    DRIVER_FILE_SYSTEM_OPEN,
    DRIVER_FILE_SYSTEM_CAN_READ,
    DRIVER_FILE_SYSTEM_CAN_WRITE,
    DRIVER_FILE_SYSTEM_READ,
    DRIVER_FILE_SYSTEM_WRITE,
    DRIVER_FILE_SYSTEM_TRUNCATE,
    DRIVER_FILE_SYSTEM_MKDIR,
    DRIVER_FILE_SYSTEM_RMDIR,

    DRIVER_FILE_SYSTEM_READ_INODE,
    DRIVER_FILE_SYSTEM_WRITE_INODE,
    DRIVER_FILE_SYSTEM_FREE_INODE,
    DRIVER_FILE_SYSTEM_GET_FSDATA,
    DRIVER_FILE_SYSTEM_LOOKUP,
    DRIVER_FILE_SYSTEM_GETDENTS,
    DRIVER_FILE_SYSTEM_CREATE,
    DRIVER_FILE_SYSTEM_UNLINK,

    DRIVER_FILE_SYSTEM_FSTAT,
    DRIVER_FILE_SYSTEM_IOCTL,
    DRIVER_FILE_SYSTEM_MMAP,
};

typedef struct {
    uint8_t type;
    bool auto_start;
    bool is_device_driver; // True if it's a dev driver (like ata driver)
    bool is_device_needed; // True if need to assign to a device
    bool is_driver_needed; // True if need to assign to a driver
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
    driver_desc_t desc;
    const char* name;
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
    uint32_t args[4];
} device_desc_t; // device desriptor

typedef struct {
    uint8_t id;
    uint8_t type;
    bool is_virtual;
    int16_t driver_id;
    device_desc_t device_desc;
} device_t; // device

extern driver_t drivers[MAX_DRIVERS_COUNT];
extern device_t devices[MAX_DEVICES_COUNT];

bool driver_manager_init();
void register_drivers();
void driver_install(driver_desc_t driver_info, const char* name);
void device_install(device_desc_t device_info);
void eject_device(uint8_t dev_id);
void eject_all_devices();
void drivers_run();
device_t get_device(uint8_t t_dev_type, uint8_t t_start);
void pass_drivers_to_master_drivers();
void pass_devices_to_master_drivers();
device_t* new_virtual_device(uint8_t type);
int dm_get_driver_id_by_name();
void dm_send_notification(uint32_t msg, uint32_t param);

static inline void* dm_driver_function(int driver_id, int function_id)
{
    return drivers[driver_id].desc.functions[function_id];
}

static inline void* dm_function_handler(device_t* dev, int function_id)
{
    return dm_driver_function(dev->driver_id, function_id);
}

#endif // _KERNEL_DRIVERS_DRIVER_MANAGER_H
