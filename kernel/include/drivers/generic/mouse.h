/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_GENERIC_MOUSE_H
#define _KERNEL_DRIVERS_GENERIC_MOUSE_H

/* The mouse packet should be aligned to 4 bytes */
struct mouse_packet {
    int16_t x_offset;
    int16_t y_offset;
    uint16_t button_states;
    int16_t wheel_data;
};
typedef struct mouse_packet mouse_packet_t;

#endif //_KERNEL_DRIVERS_GENERIC_MOUSE_H