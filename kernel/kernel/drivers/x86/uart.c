/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/x86/uart.h>
#include <platform/x86/port.h>

static int _uart_setup_impl(int port)
{
    port_byte_out(port + 1, 0x00);
    port_byte_out(port + 3, 0x80);
    port_byte_out(port + 0, 0x03);
    port_byte_out(port + 1, 0x00);
    port_byte_out(port + 3, 0x03);
    port_byte_out(port + 2, 0xC7);
    port_byte_out(port + 4, 0x0B);
    return 0;
}

void uart_setup()
{
    _uart_setup_impl(COM1);
}

static inline bool _uart_is_free_in(int port)
{
    return port_byte_in(port + 5) & 0x01;
}

static inline bool _uart_is_free_out(int port)
{
    return port_byte_in(port + 5) & 0x20;
}

int uart_write(int port, uint8_t data)
{
    while (!_uart_is_free_out(port)) { }
    port_byte_out(port, data);
    return 0;
}

int uart_read(int port, uint8_t* data)
{
    while (!_uart_is_free_out(port)) { }
    *data = port_byte_in(port);
    return 0;
}