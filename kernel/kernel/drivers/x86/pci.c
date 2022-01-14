/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/x86/pci.h>

// #define DEBUG_PCI

static driver_desc_t _pci_driver_info()
{
    driver_desc_t pci_desc = { 0 };
    pci_desc.type = DRIVER_BUS_CONTROLLER;
    pci_desc.flags = DRIVER_DESC_FLAG_START;
    pci_desc.system_funcs.on_start = pci_find_devices;
    pci_desc.functions[DRIVER_BUS_CONTROLLER_FIND_DEVICE] = pci_find_devices;
    return pci_desc;
}

static uint32_t _pci_do_read_bar(uint8_t bus, uint8_t device, uint8_t function, uint8_t bar_id)
{
    uint32_t header_type = pci_read(bus, device, function, 0x0e) & 0x7f;
    uint8_t max_bars = 6 - (header_type * 4);
    if (bar_id >= max_bars)
        return 0;

    uint32_t bar_val = pci_read(bus, device, function, 0x10 + 4 * bar_id);
    return bar_val;
}

static bar_t _pci_get_bar(uint8_t bus, uint8_t device, uint8_t function, uint8_t bar_id)
{
    bar_t result;

    uint32_t bar_val = _pci_do_read_bar(bus, device, function, bar_id);
    result.type = (bar_val & 0x1) ? INPUT_OUTPUT : MEMORY_MAPPED;

    if (result.type == MEMORY_MAPPED) {

    } else {
        result.address = (uint32_t)((bar_val >> 2) << 2);
        result.prefetchable = 0;
    }
    return result;
}

void pci_install()
{
    devman_register_driver(_pci_driver_info(), "pci86");
}
devman_register_driver_installation(pci_install);

uint32_t pci_read(uint16_t bus, uint16_t device, uint16_t function, uint32_t offset)
{
    uint32_t id = (0x1 << 31)
        | ((bus & 0xFF) << 16)
        | ((device & 0x1F) << 11)
        | ((function & 0x07) << 8)
        | (offset & 0xFC);
    port_dword_out(0xCF8, id);
    uint32_t tmp = (uint32_t)(port_dword_in(0xCFC) >> (8 * (offset % 4)));
    return tmp;
}

void pci_write(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t data)
{
    uint32_t bus32 = bus;
    uint32_t device32 = device;
    uint16_t function16 = function;
    uint32_t address = (1 << 31)
        | (bus32 << 16)
        | (device32 << 11)
        | (function16 << 8)
        | (offset & 0xFC);
    port_dword_out(0xCF8, address);
    port_dword_out(0xCFC, data);
}

char pci_has_device_functions(uint8_t bus, uint8_t device)
{
    return pci_read(bus, device, 0, 0x0e) & (1 << 7);
}

static int dev_type_by_class(device_desc_t* dd)
{
    switch (dd->pci.class_id) {
    case 0x1:
        switch (dd->pci.subclass_id) {
        case 0x1:
        case 0x3:
        case 0x4:
            return DEVICE_BUS_CONTROLLER;
        default:
            return DEVICE_STORAGE;
        }
    case 0x2:
        return DEVICE_NETWORK;
    case 0x3:
        return DEVICE_DISPLAY;
    case 0x6:
        return DEVICE_BRIDGE;
    default:
#ifdef DEBUG_PCI
        log("PCI: DEVICE_UNKNOWN: Class %d subclass %d", dd->pci.class_id, dd->pci.subclass_id);
#endif
        return DEVICE_UNKNOWN;
    }
}

int pci_find_devices()
{
#ifdef DEBUG_PCI
    log("PCI: scanning\n");
#endif
    uint8_t bus, device, function;
    for (bus = 0; bus < 8; bus++) {
        for (device = 0; device < 32; device++) {
            uint8_t functions_count = pci_has_device_functions(bus, device) == 0 ? 8 : 1;
            for (function = 0; function < functions_count; function++) {
                device_desc_t dev = pci_get_device_desriptor(bus, device, function);
                if (dev.pci.vendor_id == 0x0000 || dev.pci.vendor_id == 0xffff) {
                    continue;
                }

                for (uint8_t bar_id = 0; bar_id < 6; bar_id++) {
                    bar_t bar = _pci_get_bar(bus, device, function, bar_id);
                    if (bar.address && (bar.type == INPUT_OUTPUT)) {
                        dev.pci.port_base = (uint32_t)bar.address;
                    }
                }

                devman_register_device(dev, dev_type_by_class(&dev));
#ifdef DEBUG_PCI
                log("PCI: Vendor %x, devID %x, cl %x scl %x\n", dev.pci.vendor_id, dev.pci.device_id, dev.pci.class_id, dev.pci.subclass_id);
#endif
            }
        }
    }

    return 0;
}

device_desc_t pci_get_device_desriptor(uint8_t bus, uint8_t device, uint8_t function)
{
    device_desc_t new_device = { 0 };
    new_device.type = DEVICE_DESC_PCI;

    new_device.pci.bus = bus;
    new_device.pci.device = device;
    new_device.pci.function = function;

    new_device.pci.vendor_id = pci_read(bus, device, function, 0x00);
    new_device.pci.device_id = pci_read(bus, device, function, 0x02);

    new_device.pci.class_id = pci_read(bus, device, function, 0x0b);
    new_device.pci.subclass_id = pci_read(bus, device, function, 0x0a);
    new_device.pci.interface_id = pci_read(bus, device, function, 0x09);
    new_device.pci.revision_id = pci_read(bus, device, function, 0x08);

    new_device.pci.interrupt = pci_read(bus, device, function, 0x3c);

    return new_device;
}

uint32_t pci_read_bar(device_t* dev, int bar_id)
{
    return _pci_do_read_bar(dev->device_desc.pci.bus, dev->device_desc.pci.device, dev->device_desc.pci.function, bar_id);
}
