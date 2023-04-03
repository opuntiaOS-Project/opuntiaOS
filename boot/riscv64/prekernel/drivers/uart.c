/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "uart.h"

volatile uint32_t* output = NULL;

#define UART 0x10000000
#define UART_THR (char*)(UART + 0x00) // THR:transmitter holding register
#define UART_LSR (char*)(UART + 0x05) // LSR:line status register
#define UART_LSR_EMPTY_MASK 0x40 // LSR Bit 6: Transmitter empty; both the THR and LSR are empty

void uart_init()
{
    output = (uint32_t*)UART;
}

int uart_write(uint8_t data)
{
    if (!output) {
        return 1;
    }

    while ((*UART_LSR & UART_LSR_EMPTY_MASK) == 0) { }
    *output = data;
    return 0;
}