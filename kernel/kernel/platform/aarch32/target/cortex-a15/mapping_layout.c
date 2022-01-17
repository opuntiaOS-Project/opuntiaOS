/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <mem/vmm.h>
#include <platform/aarch32/target/cortex-a15/device_settings.h>
#include <platform/aarch32/target/cortex-a15/memmap.h>
#include <platform/generic/vmm/mapping_table.h>

mapping_entry_t kernel_mapping_table[] = {
    { .paddr = KERNEL_PM_BASE + 0x000000, .vaddr = KERNEL_BASE + 0x000000, .flags = 0, .pages = 1, .last = 0 },
    { .paddr = KERNEL_PM_BASE + 0x100000, .vaddr = KERNEL_BASE + 0x100000, .flags = 0, .pages = 1, .last = 0 },
    { .paddr = KERNEL_PM_BASE + 0x200000, .vaddr = KERNEL_BASE + 0x200000, .flags = 0, .pages = 1, .last = 0 },
    { .paddr = KERNEL_PM_BASE + 0x300000, .vaddr = KERNEL_BASE + 0x300000, .flags = 0, .pages = 1, .last = 1 },
};

mapping_entry_t extern_mapping_table[] = {
    { .paddr = UART_BASE, .vaddr = UART_BASE, .flags = PAGE_READABLE | PAGE_WRITABLE | PAGE_EXECUTABLE, .pages = 1, .last = 1 },
};