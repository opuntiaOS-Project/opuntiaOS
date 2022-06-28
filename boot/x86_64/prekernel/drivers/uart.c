/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "uart.h"
#include "port.h"

static int _uart_setup_impl(int port)
{
    port_write8(port + 1, 0x00);
    port_write8(port + 3, 0x80);
    port_write8(port + 0, 0x03);
    port_write8(port + 1, 0x00);
    port_write8(port + 3, 0x03);
    port_write8(port + 2, 0xC7);
    port_write8(port + 4, 0x0B);
    return 0;
}

void uart_init()
{
    _uart_setup_impl(COM1);
}

static inline bool _uart_is_free_in(int port)
{
    return port_read8(port + 5) & 0x01;
}

static inline bool _uart_is_free_out(int port)
{
    return port_read8(port + 5) & 0x20;
}

int uart_write(uint8_t data)
{
    while (!_uart_is_free_out(COM1)) { }
    port_write8(COM1, data);
    return 0;
}

int uart_read(uint8_t* data)
{
    while (!_uart_is_free_out(COM1)) { }
    *data = port_read8(COM1);
    return 0;
}