/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_DRIVER_MANAGER_H
#define _KERNEL_DRIVERS_DRIVER_MANAGER_H

#include <drivers/bits/device.h>
#include <drivers/bits/driver.h>
#include <drivers/devtree.h>
#include <libkern/types.h>

#define MAX_DRIVERS_COUNT 256
#define MAX_DEVICES_COUNT 64

#define DEVMAN_FUNC_NOTIFY 0x0
#define DEVMAN_FUNC_DEVICE_START 0x1
#define DEVMAN_FUNC_DRIVER_START 0x1
#define DEVMAN_FUNC_DRIVER_EMIT_DRIVER 0x1
#define DEVMAN_FUNC_DRIVER_EMIT_DEVICE 0x2

enum DEVMAN_NOTIFICATIONS {
    DEVMAN_NOTIFICATION_DEVFS_READY = 0,
    DEVMAN_NOTIFICATION_NEW_DRIVER = 1,
    DEVMAN_NOTIFICATION_NEW_DEVICE = 2,
    DEVMAN_NOTIFICATION_STOP,
};

extern driver_t drivers[MAX_DRIVERS_COUNT];
extern device_t devices[MAX_DEVICES_COUNT];

int devman_init();
void devman_run();
int devman_register_driver(driver_desc_t driver_info, const char* name);
int devman_register_device(device_desc_t device_info, int type);
device_t* new_virtual_device(int type);
int devman_get_driver_id_by_name();
void devman_send_notification(uint32_t msg, uint32_t param);

static inline void* devman_driver_function(int driver_id, int function_id)
{
    return drivers[driver_id].desc.functions[function_id];
}

static inline void* devman_function_handler(device_t* dev, int function_id)
{
    return devman_driver_function(dev->driver_id, function_id);
}

#endif // _KERNEL_DRIVERS_DRIVER_MANAGER_H
