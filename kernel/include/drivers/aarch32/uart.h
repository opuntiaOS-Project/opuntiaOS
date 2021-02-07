/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
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