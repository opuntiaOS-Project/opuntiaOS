/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <drivers/aarch32/uart.h>
#include <mem/vmm/vmm.h>
#include <mem/vmm/zoner.h>

volatile uint32_t* output = (uint32_t*)COM1;
static zone_t mapped_zone;

static inline int _uart_map_itself()
{
    mapped_zone = zoner_new_zone(VMM_PAGE_SIZE);
    vmm_map_page(mapped_zone.start, COM1, PAGE_READABLE | PAGE_WRITABLE | PAGE_EXECUTABLE);
    output = (uint32_t*)mapped_zone.ptr;
    return 0;
}

/* No setup for arm version now */
void uart_setup()
{
}

void uart_remap()
{
    _uart_map_itself();
}

int uart_write(int port, uint8_t data)
{
    *output = data;
    return 0;
}

int uart_read(int port, uint8_t* data)
{
    return 0;
}