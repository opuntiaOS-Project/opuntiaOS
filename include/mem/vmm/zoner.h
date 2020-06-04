/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#ifndef __oneOS__MEM__VMM__zoner_H
#define __oneOS__MEM__VMM__zoner_H

#include <types.h>

void zoner_init(uint32_t start_vaddr);
uint32_t zoner_new_vzone(uint32_t size);

#endif // __oneOS__MEM__VMM__zoner_H