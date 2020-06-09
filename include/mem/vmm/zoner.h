/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#ifndef __oneOS__MEM__VMM__ZONER_H
#define __oneOS__MEM__VMM__ZONER_H

#include <types.h>

struct zone
{
    uint32_t start;
    uint32_t len;
};
typedef struct zone zone_t;

void zoner_init(uint32_t start_vaddr);
void zoner_place_bitmap();

zone_t zoner_new_zone(uint32_t size);
int zoner_free_zone(zone_t zone);

#endif // __oneOS__MEM__VMM__ZONER_H