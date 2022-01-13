/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/x86/ide.h>

// ------------
// Private
// ------------

driver_desc_t _ide_driver_info();

driver_desc_t _ide_driver_info()
{
    driver_desc_t ide_desc = { 0 };
    ide_desc.type = DRIVER_BUS_CONTROLLER;
    ide_desc.listened_device_mask = DEVICE_BUS_CONTROLLER;
    ide_desc.system_funcs.init_with_dev = ide_init_with_dev;
    ide_desc.functions[DRIVER_BUS_CONTROLLER_FIND_DEVICE] = ide_init_with_dev;
    return ide_desc;
}

// ------------
// Public
// ------------

// install driver
void ide_install()
{
    devman_register_driver(_ide_driver_info(), "ide86");
}

// [Stub]
// Scanning IDE to find all drives.
// Try to recognise thier type (now by calling check function of diff techs)
int ide_init_with_dev(device_t* dev)
{
    if (dev->device_desc.type != DEVICE_DESC_PCI) {
        return -1;
    }
    if (dev->device_desc.pci.class_id != 0x01) {
        return -1;
    }
    if (dev->device_desc.pci.subclass_id != 0x01) {
        return -1;
    }

    const int DRIVES_COUNT = 2;
    uint32_t ask_ports[] = { 0x1F0, 0x1F0 };
    bool is_masters[] = { true, false };
    for (int i = 0; i < DRIVES_COUNT; i++) {
        ata_t new_drive;
        ata_init(&new_drive, ask_ports[i], is_masters[i]);
        if (ata_indentify(&new_drive)) {
            device_desc_t new_device = { 0 };
            new_device.type = DEVICE_DESC_PCI;
            new_device.pci.class_id = 0x01; // mark as storage
            new_device.pci.subclass_id = 0x05; // mark as Ata drive
            new_device.pci.interface_id = 0;
            new_device.pci.revision_id = 0;
            new_device.pci.port_base = ask_ports[i] | (1 << 31);
            new_device.pci.interrupt = IRQ14;
            devman_register_device(new_device, DEVICE_STORAGE);
        }
    }

    return 0;
}
