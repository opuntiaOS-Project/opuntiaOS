/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/aarch32/uart.h>
#include <mem/kmemzone.h>
#include <mem/vmm.h>

volatile uint32_t* output = (uint32_t*)COM1;
static kmemzone_t mapped_zone;

static inline int _uart_map_itself()
{
    mapped_zone = kmemzone_new(VMM_PAGE_SIZE);
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