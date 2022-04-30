/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/generic/screen.h>
#include <libkern/types.h>

static uintptr_t gUartBase = 0x200000000 + 0x20a0c0000;

#define rULCON0 (*(volatile uint32_t*)(gUartBase + 0x00)) // UART 0 Line control
#define rUCON0 (*(volatile uint32_t*)(gUartBase + 0x04)) // UART 0 Control
#define rUFCON0 (*(volatile uint32_t*)(gUartBase + 0x08)) // UART 0 FIFO control
#define rUMCON0 (*(volatile uint32_t*)(gUartBase + 0x0c)) // UART 0 Modem control
#define rUTRSTAT0 (*(volatile uint32_t*)(gUartBase + 0x10)) // UART 0 Tx/Rx status
#define rUERSTAT0 (*(volatile uint32_t*)(gUartBase + 0x14)) // UART 0 Rx error status
#define rUFSTAT0 (*(volatile uint32_t*)(gUartBase + 0x18)) // UART 0 FIFO status
#define rUMSTAT0 (*(volatile uint32_t*)(gUartBase + 0x1c)) // UART 0 Modem status
#define rUTXH0 (*(volatile uint32_t*)(gUartBase + 0x20)) // UART 0 Transmission Hold
#define rURXH0 (*(volatile uint32_t*)(gUartBase + 0x24)) // UART 0 Receive buffer
#define rUBRDIV0 (*(volatile uint32_t*)(gUartBase + 0x28)) // UART 0 Baud rate divisor
#define rUDIVSLOT0 (*(volatile uint32_t*)(gUartBase + 0x2C)) // UART 0 Baud rate divisor
#define rUINTM0 (*(volatile uint32_t*)(gUartBase + 0x38)) // UART 0 Baud rate divisor

void serial_putc(char c)
{
    if (c == '\n')
        serial_putc('\r');
    if (!gUartBase)
        return;
    while (!(rUTRSTAT0 & 0x04)) { }
    rUTXH0 = (unsigned)(c);
    return;
}

int uart_write(char data)
{
    serial_putc(data);
    return 0;
}

int opuntiaos_greeting()
{
    // volatile uint32_t* fb = (uint32_t*)0xfb0000000ULL;

    // uint32_t color = 0x0;
    // for (;;) {
    //     for (int i = 0; i < 3000000; i++) {
    //         fb[i] = color;
    //     }
    //     color += 0xff;
    // }

    return 0;
}