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

struct boot_args;
void uart_setup(struct boot_args* boot_args);
void uart_remap();
int uart_write(uint8_t data);
int uart_read(uint8_t* data);

#endif /* _KERNEL_DRIVERS_AARCH32_UART_H */