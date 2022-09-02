/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "drivers/fb.h"
#include "drivers/uart.h"
#include "vm.h"
#include <libboot/abi/memory.h>
#include <libboot/abi/rawimage.h>
#include <libboot/devtree/devtree.h>
#include <libboot/elf/elf_lite.h>
#include <libboot/log/log.h>
#include <libboot/mem/alloc.h>
#include <libboot/mem/mem.h>

#define DEBUG_BOOT
#define EARLY_FB

extern void jump_to_kernel(void*, uintptr_t);
static void* bootdesc_paddr;
static void* bootdesc_vaddr;
static size_t kernel_vaddr = 0;
static size_t kernel_paddr = 0;
static size_t kernel_size = 0;

#define LAUNCH_SERVER_PATH "/System/launch_server"

static int alloc_init(uintptr_t base, rawimage_header_t* riheader)
{
    devtree_entry_t* dev = devtree_find_device("ram");
    if (!dev) {
#ifdef EARLY_FB
        log("Can't find RAM in devtree");
#endif
        while (1) { };
    }

    // Currenlty we setting the allocation after the raw image.
    // Allocator is used to alloc paddrs for kernel, vm and other.
    uintptr_t start_addr = ROUND_CEIL(base + riheader->rawimage_size, page_size());
    size_t free_space = dev->region_size - (start_addr - dev->region_base);
    malloc_init((void*)start_addr, free_space);

#if defined(DEBUG_BOOT) && defined(EARLY_FB)
    log("malloc inited %llx %llx", start_addr, free_space);
#endif
    return 0;
}

static size_t memory_layout_size()
{
    return 0 + 1; // Including the trailing element.
}

static int preserve_alloc_init(size_t kernsize)
{
    const size_t devtreesize = ROUND_CEIL(devtree_size(), page_size());
    const size_t memmap = ROUND_CEIL(memory_layout_size() * sizeof(memory_layout_t), page_size());
    const size_t bootargsstruct = ROUND_CEIL(sizeof(boot_args_t), page_size());
    const size_t bootargssize = devtreesize + memmap + bootargsstruct;

    // 32 tables should be enough for initial mappings.
    const size_t prekernelvmsize = 32 * page_size();
    const size_t total = ROUND_CEIL(kernsize + bootargssize + prekernelvmsize, page_size());

    return palloc_init(total, 2 << 20);
}

static memory_boot_desc_t memory_boot_desc_init()
{
    devtree_entry_t* dev = devtree_find_device("ram");
    if (!dev) {
#ifdef EARLY_FB
        log("Can't find RAM in devtree");
#endif
        while (1) { };
    }

    memory_boot_desc_t res;
    const size_t memlayout_size = memory_layout_size();
    memory_layout_t* mem_layout_paddr = palloc_aligned(memlayout_size * sizeof(memory_layout_t), page_size());

    // Init of trailing element.
    mem_layout_paddr[memlayout_size - 1].flags = MEMORY_LAYOUT_FLAG_TERMINATE;
    memory_layout_t* mem_layout_vaddr = paddr_to_vaddr(mem_layout_paddr, kernel_paddr, kernel_vaddr);
#if defined(DEBUG_BOOT) && defined(EARLY_FB)
    log("initing MEMLAYOUT %lx of %d elems", mem_layout_vaddr, memlayout_size);
#endif

    res.ram_base = dev->region_base;
    res.ram_size = dev->region_size;
    res.reserved_areas = mem_layout_vaddr;
    return res;
}

static void load_kernel(void* kenrelstart)
{
    kernel_size = elf_get_kernel_size(kenrelstart);
    kernel_size = ROUND_CEIL(kernel_size, page_size());

    int err = preserve_alloc_init(kernel_size);
    if (err) {
#ifdef EARLY_FB
        log("add assert");
#endif
        while (1) { }
    }

    int res = elf_load_kernel(kenrelstart, kernel_size, &kernel_vaddr, &kernel_paddr);
#if defined(DEBUG_BOOT) && defined(EARLY_FB)
    log("kernel %lx %lx %lx", kernel_vaddr, kernel_paddr, kernel_size);
#endif

    void* odt_paddr = palloc_aligned(devtree_size(), page_size());
    memcpy(odt_paddr, devtree_start(), devtree_size());
    void* odt_vaddr = paddr_to_vaddr(odt_paddr, kernel_paddr, kernel_vaddr);
#if defined(DEBUG_BOOT) && defined(EARLY_FB)
    log("copying ODT %lx -> %lx of %d", devtree_start(), odt_vaddr, devtree_size());
#endif

    boot_args_t boot_args;
    boot_args.vaddr = kernel_vaddr;
    boot_args.paddr = kernel_paddr;
    boot_args.kernel_data_size = 0x0; // Sets up later
    boot_args.mem_boot_desc = memory_boot_desc_init();
    boot_args.devtree = odt_vaddr;
    boot_args.fb_boot_desc.vaddr = 0; // Marking fb as invalid.
    memcpy(boot_args.init_process, LAUNCH_SERVER_PATH, sizeof(LAUNCH_SERVER_PATH));

    bootdesc_paddr = palloc_aligned(sizeof(boot_args), page_size());
    memcpy(bootdesc_paddr, &boot_args, sizeof(boot_args));
    bootdesc_vaddr = paddr_to_vaddr(bootdesc_paddr, kernel_paddr, kernel_vaddr);
#if defined(DEBUG_BOOT) && defined(EARLY_FB)
    log("copying BOOTDESC %lx -> %lx of %d", &boot_args, bootdesc_vaddr, sizeof(boot_args));
#endif
}

static int log_merged_output(uint8_t ch)
{
    uart_write(ch);
    fb_put_char(ch);
    return 0;
}

int main(uintptr_t base, rawimage_header_t* riheader, void* devtree)
{
    devtree_init((void*)(base + riheader->devtree_off), riheader->devtree_size);
    uart_init();
    fb_init();
    log_init(log_merged_output);
    alloc_init(base, riheader);

    load_kernel((void*)(base + riheader->kern_off));
    vm_setup(base, bootdesc_paddr, riheader);

#ifdef DEBUG_BOOT
    log("Preboot done: booting to OS@%llx", ((boot_args_t*)bootdesc_vaddr)->vaddr);
#endif

    ((boot_args_t*)bootdesc_vaddr)->kernel_data_size = ROUND_CEIL(palloc_used_size(), page_size());
    jump_to_kernel((void*)bootdesc_vaddr, ((boot_args_t*)bootdesc_vaddr)->vaddr);
    return 0;
}