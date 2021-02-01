/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#ifndef __oneOS__DRIVERS__AARCH32__PL050_H
#define __oneOS__DRIVERS__AARCH32__PL050_H

#include <drivers/driver_manager.h>
#include <platform/aarch32/target/cortex-a15/device_settings.h>
#include <types.h>
#include <utils/mask.h>

struct pl050_registers {
    uint32_t cr; // control register (rw)
    uint32_t stat; // status register (r)
    uint32_t data; // data register (rw)
    uint32_t clk; // clock divisor register (rw)
    uint32_t ir;
};
typedef struct pl050_registers pl050_registers_t;

void pl050_keyboard_install();
void pl050_mouse_install();

#endif //__oneOS__DRIVERS__AARCH32__PL050_H
