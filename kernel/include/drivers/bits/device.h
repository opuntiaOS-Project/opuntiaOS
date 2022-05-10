/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_BITS_DEVICE_H
#define _KERNEL_DRIVERS_BITS_DEVICE_H

#include <libkern/types.h>

enum DEVICES_TYPE {
    DEVICE_STORAGE = (1 << 0),
    DEVICE_SOUND = (1 << 1),
    DEVICE_INPUT_SYSTEMS = (1 << 2),
    DEVICE_NETWORK = (1 << 3),
    DEVICE_DISPLAY = (1 << 4),
    DEVICE_BUS_CONTROLLER = (1 << 5),
    DEVICE_BRIDGE = (1 << 6),
    DEVICE_CHAR = (1 << 7),
    DEVICE_RTC = (1 << 8),
    DEVICE_UNKNOWN = (1 << 9),
};

enum DEVICE_DESC_TYPE {
    DEVICE_DESC_PCI,
    DEVICE_DESC_DEVTREE,
};

struct device_desc_pci {
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
};
typedef struct device_desc_pci device_desc_pci_t;

struct devtree_entry;
struct device_desc_devtree {
    struct devtree_entry* entry;
};
typedef struct device_desc_devtree device_desc_devtree_t;

struct device_desc {
    int type;
    union {
        device_desc_pci_t pci;
        device_desc_devtree_t devtree;
    };
    uint32_t args[4];
};
typedef struct device_desc device_desc_t;

#define DRIVER_ID_EMPTY (0xff)
struct device {
    int id;
    int type;
    bool is_virtual;
    int driver_id;
    device_desc_t device_desc;
};
typedef struct device device_t;

#endif // _KERNEL_DRIVERS_BITS_DEVICE_H