/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/generic/screen.h>
#include <libkern/types.h>
#include <mem/boot.h>

volatile uint8_t* uart = NULL;

void uart_setup(boot_args_t* boot_args)
{
    // This is a huuuge stub.
    // We have 2 platforms for aarch64: apl uses screen.h and
    // qemu-virt uses uart, so setting it up for it only.
    if (boot_args && boot_args->vaddr == 0x40000000) {
        uart = (uint8_t*)0x09000000;
    }
}

int uart_write(int port, uint8_t data)
{
    if (!uart) {
        return -1;
    }
    *uart = data;
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