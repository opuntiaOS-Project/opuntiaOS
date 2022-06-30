/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _BOOT_DRIVERS_UART_H
#define _BOOT_DRIVERS_UART_H

#include <libboot/types.h>

void uart_init();
int uart_write(uint8_t data);

#endif // _BOOT_DRIVERS_UART_H