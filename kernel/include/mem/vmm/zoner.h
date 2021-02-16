/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __oneOS__MEM__VMM__ZONER_H
#define __oneOS__MEM__VMM__ZONER_H

#include <types.h>

struct __zone {
    union {
        uint32_t start;
        uint8_t* ptr;
    };
    uint32_t len;
};
typedef struct __zone zone_t;

void zoner_init(uint32_t start_vaddr);
void zoner_place_bitmap();

zone_t zoner_new_zone(uint32_t size);
zone_t zoner_new_zone_aligned(uint32_t size, uint32_t alignment);
int zoner_free_zone(zone_t zone);

#endif // __oneOS__MEM__VMM__ZONER_H