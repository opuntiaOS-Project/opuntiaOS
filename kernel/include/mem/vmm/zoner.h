/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_MEM_VMM_ZONER_H
#define _KERNEL_MEM_VMM_ZONER_H

#include <libkern/types.h>

struct __zone {
    union {
        uintptr_t start;
        uint8_t* ptr;
    };
    size_t len;
};
typedef struct __zone zone_t;

void zoner_init(uint32_t start_vaddr);
void zoner_place_bitmap();

zone_t zoner_new_zone(uint32_t size);
zone_t zoner_new_zone_aligned(uint32_t size, uint32_t alignment);
int zoner_free_zone(zone_t zone);

#endif // _KERNEL_MEM_VMM_ZONER_H