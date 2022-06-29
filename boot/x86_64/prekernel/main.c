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
#include <libboot/elf/elf_lite.h>
#include <libboot/log/log.h>
#include <libboot/mem/alloc.h>
#include <libboot/types.h>

#define DEBUG_BOOT

extern void jump_to_kernel(void*, uintptr_t);
static void* bootdesc_paddr;
static void* bootdesc_vaddr;
static size_t kernel_vaddr = 0;
static size_t kernel_paddr = 0;
static size_t kernel_size = 0;

#define LAUNCH_SERVER_PATH "/System/launch_server"

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
    malloc_init((void*)start_addr, free_space);

#ifdef DEBUG_BOOT
    log("malloc inited %llx %llx", start_addr, free_space);
#endif
    return 0;
}

static size_t memory_layout_size(multiboot_info_t* multiboot)
{
    size_t n = multiboot->mmap_length / sizeof(multiboot_memory_map_t);
    return n + 1; // Including the trailing element.
}

static int preserve_alloc_init(size_t kernsize, multiboot_info_t* multiboot)
{
    const size_t memmap = ROUND_CEIL(memory_layout_size(multiboot) * sizeof(memory_layout_t), page_size());
    const size_t bootargsstruct = ROUND_CEIL(sizeof(boot_args_t), page_size());
    const size_t bootargssize = memmap + bootargsstruct;

    // 32 tables should be enough for initial mappings.
    const size_t prekernelvmsize = 32 * page_size();
    const size_t total = ROUND_CEIL(kernsize + bootargssize + prekernelvmsize, page_size());

    return palloc_init(total, 2 << 20);
}

static memory_boot_desc_t memory_boot_desc_init(multiboot_info_t* multiboot)
{
    size_t ram_last_addr = 0x0;
    size_t next_id = 0;
    memory_layout_t* mem_layout_paddr = palloc_aligned(memory_layout_size(multiboot) * sizeof(memory_layout_t), page_size());
    multiboot_memory_map_t* memmap = (multiboot_memory_map_t*)(uint64_t)multiboot->mmap_addr;
    size_t n = multiboot->mmap_length / sizeof(multiboot_memory_map_t);

    for (int i = 0; i < n; i++) {
        if (memmap[i].type != MULTIBOOT_MEMORY_AVAILABLE) {
            // The region is marked as reserved, removing it.
            mem_layout_paddr[next_id].base = memmap[i].addr;
            mem_layout_paddr[next_id].size = memmap[i].len;
            mem_layout_paddr[next_id].flags = 0;
            next_id++;
        } else {
            ram_last_addr = max(ram_last_addr, memmap[i].addr + memmap[i].len);
        }
    }
    mem_layout_paddr[next_id].flags = MEMORY_LAYOUT_FLAG_TERMINATE;
    memory_layout_t* mem_layout_vaddr = paddr_to_vaddr(mem_layout_paddr, kernel_paddr, kernel_vaddr);

    memory_boot_desc_t res;
    res.ram_base = 0x0;
    res.ram_size = ram_last_addr;
    res.reserved_areas = mem_layout_vaddr;
    return res;
}

static void load_kernel(void* kenrelstart, multiboot_info_t* multiboot)
{
    kernel_size = elf_get_kernel_size(kenrelstart);
    kernel_size = ROUND_CEIL(kernel_size, page_size());

    int err = preserve_alloc_init(kernel_size, multiboot);
    if (err) {
        log("add assert");
        while (1) { }
    }

    int res = elf_load_kernel(kenrelstart, kernel_size, &kernel_vaddr, &kernel_paddr);
#ifdef DEBUG_BOOT
    log("kernel %lx %lx %lx", kernel_vaddr, kernel_paddr, kernel_size);
#endif

    boot_args_t boot_args;
    boot_args.vaddr = kernel_vaddr;
    boot_args.paddr = kernel_paddr;
    boot_args.kernel_data_size = 0x0; // Sets up later
    boot_args.mem_boot_desc = memory_boot_desc_init(multiboot);
    boot_args.devtree = NULL;
    boot_args.fb_boot_desc.vaddr = 0; // Marking fb as invalid.
    memcpy(boot_args.init_process, LAUNCH_SERVER_PATH, sizeof(LAUNCH_SERVER_PATH));

    bootdesc_paddr = palloc_aligned(sizeof(boot_args), page_size());
    memcpy(bootdesc_paddr, &boot_args, sizeof(boot_args));
    bootdesc_vaddr = paddr_to_vaddr(bootdesc_paddr, kernel_paddr, kernel_vaddr);
#ifdef DEBUG_BOOT
    log("copying BOOTDESC %lx -> %lx of %d", &boot_args, bootdesc_vaddr, sizeof(boot_args));
#endif
}

int main(uint64_t base, multiboot_info_t* multiboot)
{
    uart_init();
    log_init(uart_write);
    alloc_init(base, multiboot);

    extern uint32_t EMBED_KERNEL_START[];
    load_kernel((void*)EMBED_KERNEL_START, multiboot);
    vm_setup(base, bootdesc_paddr);

#ifdef DEBUG_BOOT
    log("Preboot done: booting to OS@%llx", ((boot_args_t*)bootdesc_vaddr)->vaddr);
#endif

    ((boot_args_t*)bootdesc_vaddr)->kernel_data_size = ROUND_CEIL(palloc_used_size(), page_size());
    jump_to_kernel((void*)bootdesc_vaddr, ((boot_args_t*)bootdesc_vaddr)->vaddr);
    return 0;
}