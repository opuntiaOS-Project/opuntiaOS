/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#ifndef __oneOS__DRIVERS__GENERIC__MOUSE_H
#define __oneOS__DRIVERS__GENERIC__MOUSE_H

/* The mouse packet should be aligned to 4 bytes */
struct mouse_packet {
    int16_t x_offset;
    int16_t y_offset;
    uint32_t button_states;
};
typedef struct mouse_packet mouse_packet_t;

#endif //__oneOS__DRIVERS__GENERIC__MOUSE_H