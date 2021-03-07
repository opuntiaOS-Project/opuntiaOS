/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/x86/ata.h>
#include <libkern/errno.h>

ata_t _ata_drives[MAX_DEVICES_COUNT];

static uint8_t _ata_drives_count = 0;
static driver_desc_t _ata_driver_info();

static uint8_t _ata_gen_drive_head_register(bool is_lba, bool is_master, uint8_t head);

static int ata_write(device_t* device, uint32_t sector, uint8_t* data, uint32_t size);
static int ata_read(device_t* device, uint32_t sector, uint8_t* read_data);
static int ata_flush(device_t* device);
static uint32_t ata_get_capacity(device_t* device);

/**
 * Drive/Head register:
 * 1 lba, 1, drv, head [0-3]
 * drv: 0 if master else 1
 * lba: is lba access
 */
static uint8_t _ata_gen_drive_head_register(bool is_lba, bool is_master, uint8_t head)
{
    uint8_t res = 0xA0;
    res |= ((is_lba & 0x1) << 6);
    res |= ((is_master & 0x1) << 4);
    res |= (head & 15);
    return res;
}

driver_desc_t _ata_driver_info()
{
    driver_desc_t ata_desc = { 0 };
    ata_desc.type = DRIVER_STORAGE_DEVICE;
    ata_desc.auto_start = false;
    ata_desc.is_device_driver = true;
    ata_desc.is_device_needed = false;
    ata_desc.is_driver_needed = false;
    ata_desc.functions[DRIVER_NOTIFICATION] = 0;
    ata_desc.functions[DRIVER_STORAGE_ADD_DEVICE] = ata_add_new_device;
    ata_desc.functions[DRIVER_STORAGE_READ] = ata_read;
    ata_desc.functions[DRIVER_STORAGE_WRITE] = ata_write;
    ata_desc.functions[DRIVER_STORAGE_FLUSH] = ata_flush;
    ata_desc.functions[DRIVER_STORAGE_CAPACITY] = ata_get_capacity;
    ata_desc.pci_serve_class = 0x01;
    ata_desc.pci_serve_subclass = 0x05;
    ata_desc.pci_serve_vendor_id = 0x00;
    ata_desc.pci_serve_device_id = 0x00;
    return ata_desc;
}

void ata_add_new_device(device_t* new_device)
{
    bool is_master = new_device->device_desc.port_base >> 31;
    uint16_t port = new_device->device_desc.port_base & 0xFFF;
    ata_init(&_ata_drives[new_device->id], port, is_master);
    if (ata_indentify(&_ata_drives[new_device->id])) {
        kprintf("Device added to ata driver\n");
    }
}

void ata_install()
{
    // registering driver and passing info to it
    driver_install(_ata_driver_info(), "ata86");
}

void ata_init(ata_t* ata, uint32_t port, bool is_master)
{
    ata->is_master = is_master;
    ata->port.data = port;
    ata->port.error = port + 0x1;
    ata->port.sector_count = port + 0x2;
    ata->port.lba_lo = port + 0x3;
    ata->port.lba_mid = port + 0x4;
    ata->port.lba_hi = port + 0x5;
    ata->port.device = port + 0x6;
    ata->port.command = port + 0x7;
    ata->port.control = port + 0x206;
}

bool ata_indentify(ata_t* ata)
{
    port_8bit_out(ata->port.device, ata->is_master ? 0xA0 : 0xB0);
    port_8bit_out(ata->port.sector_count, 0);
    port_8bit_out(ata->port.lba_lo, 0);
    port_8bit_out(ata->port.lba_mid, 0);
    port_8bit_out(ata->port.lba_hi, 0);
    port_8bit_out(ata->port.command, 0xEC);

    // check the acceptance of a command
    uint8_t status = port_8bit_in(ata->port.command);
    if (status == 0x00) {
        kprintf("Cmd isn't accepted");
        return false;
    }

    // waiting for processing
    // while BSY is on
    while ((status & 0x80) == 0x80) {
        status = port_8bit_in(ata->port.command);
    }
    // check if drive isn't ready to transer DRQ
    if ((status & 0x08) != 0x08) {
        kprintf("Doesn't ready to transfer DRQ");
        return false;
    }

    // transfering 256 bytes of data
    for (int i = 0; i < 256; i++) {
        uint16_t data = port_16bit_in(ata->port.data);
        if (i == 1) {
            ata->cylindres = data;
        }
        if (i == 3) {
            ata->heads = data;
        }
        if (i == 6) {
            ata->sectors = data;
        }
        if (i == 49) {
            if (((data >> 8) & 0x1) == 1) {
                ata->dma = true;
            }
            if (((data >> 9) & 0x1) == 1) {
                ata->lba = true;
            }
        }

        if (i == 60) {
            ata->capacity = data;
        }
        if (i == 61) {
            ata->capacity |= ((uint32_t)data) << 16;
        }
    }

    return true;
}

int ata_write(device_t* device, uint32_t sectorNum, uint8_t* data, uint32_t size)
{
    ata_t* dev = &_ata_drives[device->id];

    uint8_t dev_config = _ata_gen_drive_head_register(true, !dev->is_master, 0);

    port_8bit_out(dev->port.device, dev_config);
    port_8bit_out(dev->port.sector_count, 1);
    port_8bit_out(dev->port.lba_lo, sectorNum & 0x000000FF);
    port_8bit_out(dev->port.lba_mid, (sectorNum & 0x0000FF00) >> 8);
    port_8bit_out(dev->port.lba_hi, (sectorNum & 0x00FF0000) >> 16);
    port_8bit_out(dev->port.error, 0);
    port_8bit_out(dev->port.command, 0x31);

    // waiting for processing
    // while BSY is on and no Errors
    uint8_t status = port_8bit_in(dev->port.command);
    while (((status >> 7) & 1) == 1 && ((status >> 0) & 1) != 1) {
        status = port_8bit_in(dev->port.command);
    }

    // check if drive isn't ready to transer DRQ
    if (((status >> 0) & 1) == 1) {
        kprintf("Error");
        return -EBUSY;
    }

    for (int i = 0; i < size; i += 2) {
        uint16_t db = (data[i + 1] << 8) + data[i];
        port_16bit_out(dev->port.data, db);
    }

    for (int i = size / 2; i < 256; i++) {
        port_16bit_out(dev->port.data, 0);
    }

    return ata_flush(device);
}

int ata_read(device_t* device, uint32_t sectorNum, uint8_t* read_data)
{
    ata_t* dev = &_ata_drives[device->id];

    uint8_t dev_config = _ata_gen_drive_head_register(true, !dev->is_master, 0);

    port_8bit_out(dev->port.device, dev_config);
    port_8bit_out(dev->port.sector_count, 1);
    port_8bit_out(dev->port.lba_lo, sectorNum & 0x000000FF);
    port_8bit_out(dev->port.lba_mid, (sectorNum & 0x0000FF00) >> 8);
    port_8bit_out(dev->port.lba_hi, (sectorNum & 0x00FF0000) >> 16);
    port_8bit_out(dev->port.error, 0);
    port_8bit_out(dev->port.command, 0x21);

    // waiting for processing
    // while BSY is on and no Errors
    uint8_t status = port_8bit_in(dev->port.command);
    while (((status >> 7) & 1) == 1 && ((status >> 0) & 1) != 1) {
        status = port_8bit_in(dev->port.command);
    }

    // check if drive isn't ready to transer DRQ
    if (((status >> 0) & 1) == 1) {
        kprintf("Error");
        return -EBUSY;
    }

    if (((status >> 3) & 1) == 0) {
        kprintf("No DRQ");
        return -ENODEV;
    }

    for (int i = 0; i < 256; i++) {
        uint16_t data = port_16bit_in(dev->port.data);
        read_data[2 * i + 1] = (data >> 8) & 0xFF;
        read_data[2 * i + 0] = (data >> 0) & 0xFF;
    }

    return 0;
}

int ata_flush(device_t* device)
{
    ata_t* dev = &_ata_drives[device->id];

    uint8_t dev_config = _ata_gen_drive_head_register(true, !dev->is_master, 0);

    port_8bit_out(dev->port.device, dev_config);
    port_8bit_out(dev->port.command, 0xE7);

    uint8_t status = port_8bit_in(dev->port.command);
    if (status == 0x00) {
        return -ENODEV;
    }

    while (((status >> 7) & 1) == 1 && ((status >> 0) & 1) != 1) {
        status = port_8bit_in(dev->port.command);
    }

    if (status & 0x01) {
        return -EBUSY;
    }

    return 0;
}

/* Returns a disk size in bytes */
uint32_t ata_get_capacity(device_t* device)
{
    // FIXME: Sector size is hard-coded.
    ata_t* dev = &_ata_drives[device->id];
    return dev->capacity * 512;
}

uint8_t ata_get_drives_count()
{
    return _ata_drives_count;
}
