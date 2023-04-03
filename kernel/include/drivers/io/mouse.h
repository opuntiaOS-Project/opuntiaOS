/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_IO_MOUSE_H
#define _KERNEL_DRIVERS_IO_MOUSE_H

#include <libkern/types.h>

/* The mouse packet should be aligned to 4 bytes */
struct mouse_packet {
    int16_t x_offset;
    int16_t y_offset;
    uint16_t button_states;
    int16_t wheel_data;
};
typedef struct mouse_packet mouse_packet_t;

int generic_mouse_create_devfs();
void generic_mouse_init();
void generic_mouse_send_packet(mouse_packet_t* packet);

#endif //_KERNEL_DRIVERS_IO_MOUSE_H