/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_X86_ATA_H
#define _KERNEL_DRIVERS_X86_ATA_H

#include <drivers/driver_manager.h>
#include <drivers/x86/display.h>
#include <libkern/types.h>
#include <mem/kmalloc.h>
#include <platform/x86/port.h>

typedef struct { // LBA28 | LBA48
    uint32_t data; // 16bit | 16 bits
    uint32_t error; // 8 bit | 16 bits
    uint32_t sector_count; // 8 bit | 16 bits
    uint32_t lba_lo; // 8 bit | 16 bits
    uint32_t lba_mid; // 8 bit | 16 bits
    uint32_t lba_hi; // 8 bit | 16 bits
    uint32_t device; // 8 bit
    uint32_t command; // 8 bit
    uint32_t control;
} ata_ports_t;

typedef struct {
    ata_ports_t port;
    bool is_master;
    uint16_t cylindres;
    uint16_t heads;
    uint16_t sectors;
    bool dma;
    bool lba;
    uint32_t capacity; // in sectors
} ata_t;

extern ata_t _ata_drives[MAX_DEVICES_COUNT];

int ata_init_with_dev(device_t* dev);

void ata_install();
void ata_init(ata_t* ata, uint32_t port, bool is_master);
bool ata_indentify(ata_t* ata);

#endif //_KERNEL_DRIVERS_X86_ATA_H
