/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_SERIAL_ARM_UART_H
#define _KERNEL_DRIVERS_SERIAL_ARM_UART_H

#include <drivers/serial/uart_api.h>
#include <libkern/types.h>

void uart_remap();

#endif /* _KERNEL_DRIVERS_SERIAL_ARM_UART_H */