/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "drivers/uart.h"
#include <libboot/abi/multiboot.h>
#include <libboot/log/log.h>

static void parse_memory_map(multiboot_info_t* multiboot)
{
    multiboot_memory_map_t* memmap = (multiboot_memory_map_t*)(uint64_t)multiboot->mmap_addr;
    size_t n = multiboot->mmap_length / sizeof(multiboot_memory_map_t);
    for (int i = 0; i < n; i++) {
        log("mem %d %lx %lx %x", i, memmap[i].addr, memmap[i].len, memmap[i].type);
    }
}

int main(uint64_t base, multiboot_info_t* multiboot)
{
    uart_init();
    log_init(uart_write);
    log("Hey from x86_64 %lx %lx", base, multiboot->mmap_length / sizeof(multiboot_memory_map_t));
    parse_memory_map(multiboot);
    while (1) { }
    return 0;
}