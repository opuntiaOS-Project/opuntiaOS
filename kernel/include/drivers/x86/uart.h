/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_X86_UART_H
#define _KERNEL_DRIVERS_X86_UART_H

#include <libkern/types.h>

#define COM1 0x3F8
#define COM2 0x2F8
#define COM3 0x3E8
#define COM4 0x2E8

struct boot_args;
void uart_setup(struct boot_args* boot_args);
int uart_write(uint8_t data);
int uart_read(uint8_t* data);

#endif //_KERNEL_DRIVERS_X86_UART_H