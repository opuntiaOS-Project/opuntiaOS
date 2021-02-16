/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __oneOS__DRIVERS__ARM_UART_H
#define __oneOS__DRIVERS__ARM_UART_H

#include <types.h>

#define COM1 0x1c090000

void uart_setup();
void uart_remap();
int uart_write(int port, uint8_t data);
int uart_read(int port, uint8_t* data);

#endif /* __oneOS__DRIVERS__ARM_UART_H */