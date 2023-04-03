/*
 * Copyright (C) 2020-2023 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "vm.h"
#include <libboot/abi/kernel.h>
#include <libboot/abi/rawimage.h>
#include <libboot/devtree/devtree.h>
#include <libboot/log/log.h>
#include <libboot/mem/alloc.h>
#include <libboot/mem/mem.h>

#define DEBUG_VM

static uint64_t* global_page_table;
static const uint64_t kernel_base = 0xffff800000000000;

static uint64_t* new_ptable(boot_args_t* args)
{
    uint64_t* res = (uint64_t*)palloc_aligned(page_size(), page_size());
    memset(res, 0, page_size());
#ifdef DEBUG_VM
    log("   alloc ptable %llx %llx", (uint64_t)res, page_size());
#endif
    return res;
}

static void map4kb_2mb(boot_args_t* args, size_t phyz, size_t virt)
{
    const size_t page_covers = (1ull << PTABLE_LV1_VADDR_OFFSET);
    const size_t page_mask = page_covers - 1;

    if ((phyz & page_mask) != 0 || (virt & page_mask) != 0) {
        return;
    }

    // Mapping from level3.
    uint64_t* page_table = global_page_table;
    uint64_t ptable_desc = page_table[VM_VADDR_OFFSET_AT_LEVEL(virt, PTABLE_LV3_VADDR_OFFSET, VMM_LV3_ENTITY_COUNT)];
    if (ptable_desc == 0) {
        uint64_t* nptbl = new_ptable(args);
        uint64_t pdesc = 0x00000000000001; // v bit is set
        pdesc |= ((uintptr_t)nptbl >> 2);
        page_table[VM_VADDR_OFFSET_AT_LEVEL(virt, PTABLE_LV3_VADDR_OFFSET, VMM_LV3_ENTITY_COUNT)] = pdesc;
        ptable_desc = pdesc;
    }

    // Level2
    page_table = (uint64_t*)(((ptable_desc >> 10) << 12) & 0xffffffffffff);
    ptable_desc = page_table[VM_VADDR_OFFSET_AT_LEVEL(virt, PTABLE_LV2_VADDR_OFFSET, VMM_LV2_ENTITY_COUNT)];
    if (ptable_desc == 0) {
        uint64_t* nptbl = new_ptable(args);
        uint64_t pdesc = 0x00000000000001; // v bit is set
        pdesc |= ((uintptr_t)nptbl >> 2);
        page_table[VM_VADDR_OFFSET_AT_LEVEL(virt, PTABLE_LV2_VADDR_OFFSET, VMM_LV2_ENTITY_COUNT)] = pdesc;
        ptable_desc = pdesc;
    }

    page_table = (uint64_t*)(((ptable_desc >> 10) << 12) & 0xffffffffffff);
    uint64_t pdesc = 0x0000000000000f; // w + v + x + r bits are set.
    pdesc |= ((uintptr_t)phyz >> 2);
    page_table[VM_VADDR_OFFSET_AT_LEVEL(virt, PTABLE_LV1_VADDR_OFFSET, VMM_LV1_ENTITY_COUNT)] = pdesc;
}

static void map4kb_1gb(boot_args_t* args, size_t phyz, size_t virt)
{
    const size_t page_covers = (1ull << PTABLE_LV2_VADDR_OFFSET);
    const size_t page_mask = page_covers - 1;

    if ((phyz & page_mask) != 0 || (virt & page_mask) != 0) {
        return;
    }

    // Mapping from level3.
    uint64_t* page_table = global_page_table;
    uint64_t ptable_desc = page_table[VM_VADDR_OFFSET_AT_LEVEL(virt, PTABLE_LV3_VADDR_OFFSET, VMM_LV3_ENTITY_COUNT)];
    if (ptable_desc == 0) {
        uint64_t* nptbl = new_ptable(args);
        uint64_t pdesc = 0x00000000000001; // v bit is set
        pdesc |= ((uintptr_t)nptbl >> 2);
        page_table[VM_VADDR_OFFSET_AT_LEVEL(virt, PTABLE_LV3_VADDR_OFFSET, VMM_LV3_ENTITY_COUNT)] = pdesc;
        ptable_desc = pdesc;
    }

    page_table = (uint64_t*)(((ptable_desc >> 10) << 12) & 0xffffffffffff);
    uint64_t pdesc = 0x0000000000000f; // w + v + x + r bits are set.
    pdesc |= ((uintptr_t)phyz >> 2);
    page_table[VM_VADDR_OFFSET_AT_LEVEL(virt, PTABLE_LV2_VADDR_OFFSET, VMM_LV2_ENTITY_COUNT)] = pdesc;
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

void vm_setup(uintptr_t base, boot_args_t* args)
{
    global_page_table = (uint64_t*)palloc_aligned(page_size(), page_size());
    memset(global_page_table, 0, page_size());

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

    map_uart(args);

#ifdef DEBUG_VM
    log("Complete tables setup");
#endif

    extern void enable_mmu(uintptr_t, uintptr_t);
    enable_mmu((uintptr_t)(9ull << 60) | (((uintptr_t)global_page_table >> 12) & 0xffffffffff), (1 << 18));
}
