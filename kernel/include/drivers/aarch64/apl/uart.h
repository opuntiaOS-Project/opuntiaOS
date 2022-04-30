/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_AARCH64_APL_UART_H
#define _KERNEL_DRIVERS_AARCH64_APL_UART_H

#include <libkern/types.h>

// Fake UART for a cpu 10X
#define COM1 (0x0)

static inline void uart_setup() { }
static inline void uart_remap() { }
static inline int uart_write(int port, uint8_t data) { return -1; }
static inline int uart_read(int port, uint8_t* data) { return -1; }

#endif /* _KERNEL_DRIVERS_AARCH64_APL_UART_H */