/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <mem/vmm.h>
#include <platform/arm32/target/cortex-a15/device_settings.h>
#include <platform/arm32/target/cortex-a15/memmap.h>
#include <platform/generic/vmm/mapping_table.h>

mapping_entry_t extern_mapping_table[] = {
    { .paddr = UART_BASE, .vaddr = UART_BASE, .flags = MMU_FLAG_PERM_READ | MMU_FLAG_PERM_WRITE | MMU_FLAG_PERM_EXEC, .pages = 1, .last = 1 },
};