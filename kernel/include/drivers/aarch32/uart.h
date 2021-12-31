/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_AARCH32_UART_H
#define _KERNEL_DRIVERS_AARCH32_UART_H

#include <libkern/types.h>

#define COM1 0x1c090000

void uart_setup();
void uart_remap();
int uart_write(int port, uint8_t data);
int uart_read(int port, uint8_t* data);

#endif /* _KERNEL_DRIVERS_AARCH32_UART_H */