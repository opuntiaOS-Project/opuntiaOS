/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
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

void uart_setup();
int uart_write(int port, uint8_t data);
int uart_read(int port, uint8_t* data);

#endif /* __oneOS__DRIVERS__UART_H */