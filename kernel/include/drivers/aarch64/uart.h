/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_AARCH64_UART_H
#define _KERNEL_DRIVERS_AARCH64_UART_H

#include <libkern/types.h>

struct boot_args;
void uart_setup(struct boot_args* boot_args);
static inline void uart_remap() { }
int uart_write(uint8_t data);
static inline int uart_read(uint8_t* data) { return -1; }

#endif // _KERNEL_DRIVERS_AARCH64_UART_H