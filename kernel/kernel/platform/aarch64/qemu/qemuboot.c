/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libkern/libkern.h>
#include <platform/aarch64/system.h>
#include <mem/boot.h>

static memory_map_t qemu_virt_memmap[1] = {
    {
        .startLo = 0x40000000, // 2GB
        .startHi = 0x0,
        .sizeLo = 256 << 20, // 256MB
        .sizeHi = 0x0,
        .type = 0x1, // Free
        .acpi_3_0 = 0x0,
    },
};

static boot_args_t qemu_boot_args = {
    .paddr = 0x40000000,
    .vaddr = 0x40000000,
    .kernel_size = 4 << 20, // 4mb is enough.
    .memory_map = &qemu_virt_memmap,
    .memory_map_size = 1,
    .devtree = NULL,
    .fb_boot_desc.vaddr = 0,
};

// For QEMU-virt the boot env is simulated.
boot_args_t* qemu_virt_boot_init()
{
    return &qemu_boot_args;
}
