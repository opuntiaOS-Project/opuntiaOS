/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/devtree.h>
#include <drivers/driver_manager.h>
#include <libkern/libkern.h>
#include <mem/kmalloc.h>

static devtree_header_t* devtree_header = NULL;
static devtree_entry_t* devtree_body = NULL;
static char* devtree_name_section = NULL;
static const char* devtree_virt_name_sections[16];
static uint32_t devtree_next_virt_name_section = 0x0;

// DEVICE_UNKNOWN aren't passed to devman_register_device.
static int devtree_type_to_devman_type[] = {
    [DEVTREE_ENTRY_TYPE_IO] = DEVICE_INPUT_SYSTEMS,
    [DEVTREE_ENTRY_TYPE_STORAGE] = DEVICE_STORAGE,
    [DEVTREE_ENTRY_TYPE_BUS_CONTROLLER] = DEVICE_BUS_CONTROLLER,
    [DEVTREE_ENTRY_TYPE_RTC] = DEVICE_RTC,
    [DEVTREE_ENTRY_TYPE_FB] = DEVICE_DISPLAY,
    [DEVTREE_ENTRY_TYPE_UART] = DEVICE_CHAR,
    [DEVTREE_ENTRY_TYPE_RAM] = DEVICE_UNKNOWN,
};

static int find_devices()
{
    for (int i = 0; i < devtree_header->entries_count; i++) {
        int devtype = devtree_type_to_devman_type[devtree_body[i].type];
        if (devtype == DEVICE_UNKNOWN) {
            continue;
        }

        device_desc_t new_device = { 0 };
        new_device.type = DEVICE_DESC_DEVTREE;
        new_device.devtree.entry = &devtree_body[i];
        devman_register_device(new_device, devtype);
    }

    return 0;
}

static driver_desc_t _devtree_driver_info()
{
    driver_desc_t dt_desc = { 0 };
    dt_desc.type = DRIVER_VIRTUAL;
    dt_desc.flags = DRIVER_DESC_FLAG_START;
    dt_desc.system_funcs.on_start = find_devices;
    return dt_desc;
}

int devtree_init(boot_args_t* boot_args)
{
    void* devtree = boot_args->devtree;
    if (!devtree) {
        return 1;
    }

    devtree_header_t* dth = (devtree_header_t*)devtree;
    if (memcmp(dth->signature, DEVTREE_HEADER_SIGNATURE, DEVTREE_HEADER_SIGNATURE_LEN)) {
        return 1;
    }

    devtree_header = dth;
    devtree_body = (devtree_entry_t*)&dth[1];
    devtree_name_section = ((char*)dth + dth->name_list_offset);
    devman_register_driver(_devtree_driver_info(), "devtree");
    return 0;
}

const char* devtree_name_of_entry(devtree_entry_t* en)
{
    if (&devtree_body[0] <= en && en <= &devtree_body[devtree_header->entries_count]) {
        return &devtree_name_section[en->rel_name_offset];
    }
    return devtree_virt_name_sections[en->rel_name_offset];
}

devtree_entry_t* devtree_find_device(const char* name)
{
    if (!devtree_body) {
        return NULL;
    }

    for (int i = 0; i < devtree_header->entries_count; i++) {
        const char* curdev_name = devtree_name_of_entry(&devtree_body[i]);
        if (strcmp(curdev_name, name) == 0) {
            return &devtree_body[i];
        }
    }
    return NULL;
}

uint32_t devtree_new_entry_name(const char* ptr)
{
    uint32_t nxt = devtree_next_virt_name_section++;
    devtree_virt_name_sections[nxt] = ptr;
    return nxt;
}

devtree_entry_t* devtree_new_entry(const devtree_entry_t* from)
{
    devtree_entry_t* new_entry = kmalloc(sizeof(devtree_entry_t));
    memcpy(new_entry, from, sizeof(devtree_entry_t));
    return new_entry;
}