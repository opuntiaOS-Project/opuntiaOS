/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
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
    ide_desc.auto_start = false;
    ide_desc.is_device_driver = true;
    ide_desc.is_device_needed = false;
    ide_desc.is_driver_needed = false;
    ide_desc.functions[DRIVER_NOTIFICATION] = 0;
    ide_desc.functions[DRIVER_BUS_CONTROLLER_FIND_DEVICE] = ide_find_devices;
    ide_desc.pci_serve_class = 0x01;
    ide_desc.pci_serve_subclass = 0x01;
    ide_desc.pci_serve_vendor_id = 0x00;
    ide_desc.pci_serve_device_id = 0x00;
    return ide_desc;
}

// ------------
// Public
// ------------

// install driver
void ide_install()
{
    driver_install(_ide_driver_info(), "ide86");
}

// [Stub]
// Scanning IDE to find all drives.
// Try to recognise thier type (now by calling check function of diff techs)
void ide_find_devices(device_t* t_device)
{
    const uint8_t DRIVES_COUNT = 2;
    uint32_t ask_ports[] = { 0x1F0, 0x1F0 };
    bool is_masters[] = { true, false };
    for (uint8_t i = 0; i < DRIVES_COUNT; i++) {
        ata_t new_drive;
        ata_init(&new_drive, ask_ports[i], is_masters[i]);
        if (ata_indentify(&new_drive)) {
            device_desc_t new_device = { 0 };
            new_device.class_id = 0x01; // mark as storage
            new_device.subclass_id = 0x05; // mark as Ata drive
            new_device.interface_id = 0;
            new_device.revision_id = 0;
            new_device.port_base = ask_ports[i] | (1 << 31);
            new_device.interrupt = IRQ14;
            device_install(new_device);
        }
    }
}
