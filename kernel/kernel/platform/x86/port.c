/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/x86/port.h>

uint8_t port_read8(uint16_t port)
{
    uint8_t result_data;
    asm volatile("inb %%dx, %%al"
                 : "=a"(result_data)
                 : "d"(port));
    return result_data;
}

void port_write8(uint16_t port, uint8_t data)
{
    asm volatile("outb %%al, %%dx"
                 :
                 : "a"(data), "d"(port));
}

uint16_t port_read16(uint16_t port)
{
    uint16_t result_data;
    asm volatile("inw %%dx, %%ax"
                 : "=a"(result_data)
                 : "d"(port));
    return result_data;
}

void port_write16(uint16_t port, uint16_t data)
{
    asm volatile("outw %%ax, %%dx"
                 :
                 : "a"(data), "d"(port));
}

uint32_t port_read32(uint16_t port)
{
    uint32_t result_data;
    asm volatile("inl %%dx, %%eax"
                 : "=a"(result_data)
                 : "d"(port));
    return result_data;
}

void port_write32(uint16_t port, uint32_t data)
{
    asm volatile("outl %%eax, %%dx"
                 :
                 : "a"(data), "d"(port));
}

void port_wait_io()
{
    asm volatile("out %%al, $0x80"
                 :
                 : "a"(0)); // writing to "unused" port
}