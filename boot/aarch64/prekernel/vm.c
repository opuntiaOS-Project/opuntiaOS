/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "vm.h"
#include "drivers/fb.h"
#include <libboot/abi/kernel.h>
#include <libboot/abi/rawimage.h>
#include <libboot/devtree/devtree.h>
#include <libboot/log/log.h>
#include <libboot/mem/alloc.h>
#include <libboot/mem/mem.h>

// #define DEBUG_VM

static uint64_t* global_page_table_0;
static uint64_t* global_page_table_1;

// This is setup for 4KB pages
static const int tg0 = 0b00;
static const int tg1 = 0b10;
static const int t0sz = 25;
static const int t1sz = 25;
static const uint64_t kernel_base = 0xffffffffffffffff - ((1ull << (64 - t1sz)) - 1);

static uint64_t* new_ptable(boot_args_t* args)
{
    uint64_t* res = (uint64_t*)palloc_aligned(page_size(), page_size());
    memset(res, 0, page_size());
#ifdef DEBUG_VM
    log("   alloc ptable %llx %llx", (uint64_t)res, page_size());
#endif
    return res;
}

// Huge page impl is not suitable for Apl, where we need percise mappings.
static void map4kb_1gb(boot_args_t* args, size_t phyz, size_t virt)
{
    const size_t page_covers = (1ull << PTABLE_LV2_VADDR_OFFSET);
    const size_t page_mask = page_covers - 1;

    if ((phyz & page_mask) != 0 || (virt & page_mask) != 0) {
        return;
    }

    // Mapping from level2, as needed.
    uint64_t* page_table = global_page_table_0;
    if (virt >= kernel_base) {
        page_table = global_page_table_1;
        virt -= kernel_base;
    }

    uint64_t pdesc = 0x00000000000701;
    pdesc |= (uintptr_t)phyz;
    page_table[VM_VADDR_OFFSET_AT_LEVEL(virt, PTABLE_LV2_VADDR_OFFSET, VMM_LV2_ENTITY_COUNT)] = pdesc;
}

static void map4kb_2mb(boot_args_t* args, size_t phyz, size_t virt)
{
    const size_t page_covers = (1ull << PTABLE_LV1_VADDR_OFFSET);
    const size_t page_mask = page_covers - 1;

    if ((phyz & page_mask) != 0 || (virt & page_mask) != 0) {
        return;
    }

    // Mapping from level2, as needed.
    uint64_t* page_table = global_page_table_0;
    if (virt >= kernel_base) {
        page_table = global_page_table_1;
        virt -= kernel_base;
    }

    uint64_t ptable_desc = page_table[VM_VADDR_OFFSET_AT_LEVEL(virt, PTABLE_LV2_VADDR_OFFSET, VMM_LV2_ENTITY_COUNT)];
    if (ptable_desc == 0) {
        uint64_t* nptbl = new_ptable(args);
        uint64_t pdesc = 0x00000000000003;
        pdesc |= (uintptr_t)nptbl;
        page_table[VM_VADDR_OFFSET_AT_LEVEL(virt, PTABLE_LV2_VADDR_OFFSET, VMM_LV2_ENTITY_COUNT)] = pdesc;
        ptable_desc = pdesc;
    }

    page_table = (uint64_t*)(((ptable_desc >> 12) << 12) & 0xffffffffffff);
    uint64_t pdesc = 0x00000000000701;
    pdesc |= (uintptr_t)phyz;
    page_table[VM_VADDR_OFFSET_AT_LEVEL(virt, PTABLE_LV1_VADDR_OFFSET, VMM_LV1_ENTITY_COUNT)] = pdesc;
}

static void map_uart(boot_args_t* args)
{
    devtree_entry_t* dev = devtree_find_device("uart");
    if (!dev) {
        return;
    }

    uint64_t paddr = dev->region_base;
    const size_t page_covers = (1ull << PTABLE_LV2_VADDR_OFFSET);
    paddr &= ~(page_covers - 1);

#ifdef DEBUG_VM
    log("mapping %lx %lx", paddr, paddr);
#endif
    map4kb_1gb(args, paddr, paddr);
}

static void map_fb(boot_args_t* args)
{
    devtree_entry_t* dev = devtree_find_device("aplfb");
    if (!dev) {
        return;
    }

    uint64_t paddr = dev->region_base;
    const size_t page_covers = (1ull << PTABLE_LV2_VADDR_OFFSET);
    paddr &= ~(page_covers - 1);

#ifdef DEBUG_VM
    log("mapping %lx %lx", paddr, 0xfc0000000ULL);
#endif
    map4kb_1gb(args, paddr, 0xfc0000000ULL);

    args->fb_boot_desc.paddr = dev->region_base;
    args->fb_boot_desc.vaddr = 0xfc0000000ULL + (args->fb_boot_desc.paddr - paddr);
    args->fb_boot_desc.width = dev->aux1;
    args->fb_boot_desc.height = dev->aux2;
    args->fb_boot_desc.pixels_per_row = dev->aux3;
}

void vm_setup(uintptr_t base, boot_args_t* args, rawimage_header_t* riheader)
{
    // This implementation is a stub, supporting only 4kb pages for now.
    // We should support 16kb pages for sure on modern Apls.
    global_page_table_0 = (uint64_t*)palloc_aligned(page_size(), page_size());
    memset(global_page_table_0, 0, page_size());

    global_page_table_1 = (uint64_t*)palloc_aligned(page_size(), page_size());
    memset(global_page_table_1, 0, page_size());

    const size_t map_range_2mb = (2 << 20);
    const size_t map_range_1gb = (1 << 30);

    // Mapping kernel vaddr to paddr
    size_t kernel_size_to_map = palloc_total_size() + shadow_area_size();
    size_t kernel_range_count_to_map = (kernel_size_to_map + (map_range_2mb - 1)) / map_range_2mb;
    for (size_t i = 0; i < kernel_range_count_to_map; i++) {
#ifdef DEBUG_VM
        log("mapping %lx %lx", args->paddr + i * map_range_2mb, args->vaddr + i * map_range_2mb);
#endif
        map4kb_2mb(args, args->paddr + i * map_range_2mb, args->vaddr + i * map_range_2mb);
    }

    // Mapping RAM
    size_t ram_base = ROUND_FLOOR(args->mem_boot_desc.ram_base, map_range_1gb);
    size_t ram_size = args->mem_boot_desc.ram_size;
    size_t ram_range_count_to_map = (ram_size + (map_range_1gb - 1)) / map_range_1gb;
    for (size_t i = 0; i < ram_range_count_to_map; i++) {
#ifdef DEBUG_VM
        log("mapping %lx %lx", ram_base + i * map_range_1gb, ram_base + i * map_range_1gb);
#endif
        map4kb_1gb(args, ram_base + i * map_range_1gb, ram_base + i * map_range_1gb);
    }

    // The initial boot requires framebuffer and uart to be mapped.
    // Checking this and mapping devices.
    map_uart(args);
    map_fb(args);

    extern void enable_mmu_el1(uint64_t ttbr0, uint64_t tcr, uint64_t mair, uint64_t ttbr1);
    enable_mmu_el1((uint64_t)global_page_table_0, 0x135003500 | (tg0 << 14) | (tg1 << 30) | (t1sz << 16) | t0sz, 0x04ff, (uint64_t)global_page_table_1);

    fb_reinit_after_map(args->fb_boot_desc.vaddr);
}
