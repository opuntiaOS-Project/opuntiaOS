/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../memmap.h"

memory_map_t arm_memmap[2] = {
    {
        .startLo = 0x0,
        .startHi = 0x0,
        .sizeLo = 0x80000000, // 2GB
        .sizeHi = 0x0,
        .type = 0x0,
        .acpi_3_0 = 0x0,
    },
    {
        .startLo = 0x80000000, // 2GB
        .startHi = 0x0,
        .sizeLo = 0x7FFFFFFF, // 2GB
        .sizeHi = 0x0,
        .type = 0x1, // Free
        .acpi_3_0 = 0x0,
    },
};