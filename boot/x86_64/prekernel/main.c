/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "drivers/uart.h"
#include "vm.h"
#include <libboot/abi/multiboot.h>
#include <libboot/log/log.h>
#include <libboot/types.h>

// #define DEBUG_BOOT

static void parse_memory_map(multiboot_info_t* multiboot)
{
    multiboot_memory_map_t* memmap = (multiboot_memory_map_t*)(uint64_t)multiboot->mmap_addr;
    size_t n = multiboot->mmap_length / sizeof(multiboot_memory_map_t);
    for (int i = 0; i < n; i++) {
        log("mem %d %lx %lx %x", i, memmap[i].addr, memmap[i].len, memmap[i].type);
    }
}

static int alloc_init(uintptr_t base, multiboot_info_t* multiboot)
{
    uintptr_t region_base = 0x0;
    size_t region_size = 0x0;

    multiboot_memory_map_t* memmap = (multiboot_memory_map_t*)(uint64_t)multiboot->mmap_addr;
    size_t n = multiboot->mmap_length / sizeof(multiboot_memory_map_t);

    // Looking for the zone of memory where we are linked
    for (int i = 0; i < n; i++) {
        if (memmap[i].type == MULTIBOOT_MEMORY_AVAILABLE) {
            if (memmap[i].addr <= base && base < memmap[i].addr + memmap[i].len) {
                region_base = memmap[i].addr;
                region_size = memmap[i].len;
            }
        }
    }

    // We expect the current zone to be quite big, at least 128Mb.
    if (region_size < (128 << 20)) {
        log("Current space is less than required 128Mbs");
        while (1) { }
    }

    extern uint32_t RAWIMAGE_END[];
    uintptr_t start_addr = ROUND_CEIL((uint64_t)RAWIMAGE_END, page_size());
    size_t free_space = region_size - (start_addr - region_base);

#ifdef DEBUG_BOOT
    log("malloc inited %llx %llx", start_addr, free_space);
#endif
    return 0;
}

int main(uint64_t base, multiboot_info_t* multiboot)
{
    uart_init();
    log_init(uart_write);
    alloc_init(base, multiboot);
    log("Hey from x86_64 %lx %lx", base, multiboot->mmap_length / sizeof(multiboot_memory_map_t));
    parse_memory_map(multiboot);
    while (1) { }
    return 0;
}