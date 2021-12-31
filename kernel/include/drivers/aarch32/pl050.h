/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_AARCH32_PL050_H
#define _KERNEL_DRIVERS_AARCH32_PL050_H

#include <drivers/driver_manager.h>
#include <libkern/mask.h>
#include <libkern/types.h>
#include <platform/aarch32/target/cortex-a15/device_settings.h>

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

#endif //_KERNEL_DRIVERS_AARCH32_PL050_H
