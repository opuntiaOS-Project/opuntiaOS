/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_BITS_DRIVER_H
#define _KERNEL_DRIVERS_BITS_DRIVER_H

#include <libkern/types.h>

#define MAX_DRIVER_FUNCTION_COUNT 32

// Supported driver's types
enum DRIVERS_TYPE {
    DRIVER_STORAGE_DEVICE = (1 << 0),
    DRIVER_VIDEO_DEVICE = (1 << 1),
    DRIVER_SOUND_DEVICE = (1 << 2),
    DRIVER_INPUT_SYSTEMS_DEVICE = (1 << 3),
    DRIVER_NETWORK_DEVICE = (1 << 4),
    DRIVER_BUS_CONTROLLER = (1 << 5),
    DRIVER_VIRTUAL_FILE_SYSTEM = (1 << 6),
    DRIVER_FILE_SYSTEM = (1 << 7),
    DRIVER_VIRTUAL = (1 << 8),
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

struct driver;
struct device;
typedef struct {
    void (*recieve_notification)(uint32_t key, uint32_t val);
    int (*on_start)();
    int (*on_stop)();
    int (*init_with_dev)(struct device* device);
} driver_system_funcs_t;

enum DRIVER_DESC_FLAGS {
    DRIVER_DESC_FLAG_START = (1 << 0),
};

struct driver_desc {
    int type;
    uint32_t flags;
    uint32_t listened_device_mask;
    uint32_t listened_driver_mask;
    void* functions[MAX_DRIVER_FUNCTION_COUNT];
    driver_system_funcs_t system_funcs;
};
typedef struct driver_desc driver_desc_t;

struct driver {
    int id;
    int status;
    driver_desc_t desc;
    const char* name;
};
typedef struct driver driver_t;

#endif // _KERNEL_DRIVERS_BITS_DRIVER_H