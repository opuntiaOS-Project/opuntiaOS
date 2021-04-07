/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <mem/vmm/vmm.h>
#include <platform/generic/vmm/mapping_table.h>
#include <platform/x86/memmap.h>

mapping_entry_t kernel_mapping_table[] = {
    { .paddr = KERNEL_PM_BASE, .vaddr = KERNEL_BASE, .flags = 0, .pages = 1, .last = 0 },
    { .paddr = BIOS_SETTING_PM_BASE, .vaddr = BIOS_SETTING_BASE, .flags = 0, .pages = 1, .last = 1 },
};

mapping_entry_t extern_mapping_table[] = {
    { .paddr = 0x00000000, .vaddr = 0x00000000, .flags = PAGE_READABLE | PAGE_WRITABLE | PAGE_EXECUTABLE, .pages = 1024, .last = 1 },
};