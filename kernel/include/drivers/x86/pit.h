/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#ifndef __oneOS__DRIVERS__PIT_H
#define __oneOS__DRIVERS__PIT_H

#include <types.h>
#include <platform/x86/idt.h>

#define PIT_BASE_FREQ 1193180
#define PIT_TICKS_PER_SECOND 125

void pit_setup();
void pit_handler();

#endif /* __oneOS__DRIVERS__PIT_H */

