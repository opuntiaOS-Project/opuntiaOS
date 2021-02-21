/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __oneOS__DRIVERS__PL031_H
#define __oneOS__DRIVERS__PL031_H

#include <drivers/driver_manager.h>
#include <platform/aarch32/interrupts.h>
#include <platform/aarch32/target/cortex-a15/device_settings.h>
#include <types.h>
#include <utils/mask.h>

struct pl031_registers {
    uint32_t data;
    uint32_t match;
    uint32_t load;
    uint32_t control;
    // TO BE CONTINUED
};
typedef struct pl031_registers pl031_registers_t;

void pl031_install();
uint32_t pl031_read_rtc();

#endif // __oneOS__DRIVERS__PL031_H