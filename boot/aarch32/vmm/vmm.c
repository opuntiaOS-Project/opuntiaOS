/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "vmm.h"
#include <libboot/log/log.h>
#include <libboot/mem/alloc.h>
#include <libboot/mem/mem.h>
#include <libboot/types.h>

// #define DEBUG_VMM

#define VMM_OFFSET_IN_DIRECTORY(a) (((a) >> 20) & 0xfff)
#define VMM_OFFSET_IN_TABLE(a) (((a) >> 12) & 0xff)
#define VMM_OFFSET_IN_PAGE(a) ((a)&0xfff)
#define ALIGN_TO_TABLE(a) ((a)&0xfff00000)

static pdirectory_t* pdir = NULL;

static ptable_t* map_table();
static void mmu_enable();

static inline void write_ttbcr(uint32_t val)
{
    asm volatile("mcr p15, 0, %0, c2, c0, 2"
                 :
                 : "r"(val)
                 : "memory");
    asm volatile("dmb");
}

static inline void write_ttbr0(uint32_t val)
{
    asm volatile("mcr p15, 0, %0, c2, c0, 0"
                 :
                 : "r"(val)
                 : "memory");
    asm volatile("dmb");
}

static inline void write_dacr(uint32_t val)
{
    asm volatile("mcr p15, 0, %0, c3, c0, 0"
                 :
                 : "r"(val));
    asm volatile("dmb");
}

static void mmu_enable()
{
    volatile uint32_t val;
    asm volatile("mrc p15, 0, %0, c1, c0, 0"
                 : "=r"(val));
    asm volatile("orr %0, %1, #0x1"
                 : "=r"(val)
                 : "r"(val));
    asm volatile("mcr p15, 0, %0, c1, c0, 0" ::"r"(val)
                 : "memory");
    asm volatile("isb");
}

static pdirectory_t* vm_alloc_pdir()
{
    return (pdirectory_t*)malloc_aligned(sizeof(pdirectory_t), sizeof(pdirectory_t));
}

static ptable_t* vm_alloc_ptable()
{
    return (ptable_t*)malloc_aligned(sizeof(ptable_t), sizeof(ptable_t));
}

static ptable_t* map_table(size_t tphyz, size_t tvirt)
{
    ptable_t* table = vm_alloc_ptable();
    for (size_t phyz = tphyz, virt = tvirt, i = 0; i < VMM_LV0_ENTITY_COUNT; phyz += VMM_PAGE_SIZE, virt += VMM_PAGE_SIZE, i++) {
        page_desc_t new_page;
        new_page.one = 1;
        new_page.baddr = (phyz / VMM_PAGE_SIZE);
        new_page.tex = 0b001;
        new_page.c = 1;
        new_page.b = 1;
        new_page.ap1 = 0b11;
        new_page.ap2 = 0b0;
        new_page.s = 1;
        table->entities[i] = new_page;
    }

    uint32_t table_int = (uint32_t)table;
    pdir->entities[VMM_OFFSET_IN_DIRECTORY(tvirt)].valid = 1;
    pdir->entities[VMM_OFFSET_IN_DIRECTORY(tvirt)].zero1 = 0;
    pdir->entities[VMM_OFFSET_IN_DIRECTORY(tvirt)].zero2 = 0;
    pdir->entities[VMM_OFFSET_IN_DIRECTORY(tvirt)].ns = 0;
    pdir->entities[VMM_OFFSET_IN_DIRECTORY(tvirt)].zero3 = 0;
    pdir->entities[VMM_OFFSET_IN_DIRECTORY(tvirt)].domain = 0b0011;
    pdir->entities[VMM_OFFSET_IN_DIRECTORY(tvirt)].baddr = ((table_int / 1024));

    return table;
}

static void vm_init()
{
    pdir = vm_alloc_pdir();

    for (int i = 0; i < VMM_LV1_ENTITY_COUNT; i++) {
        pdir->entities[i].valid = 0;
    }
}

static void vm_map_devices()
{
    map_table(0x1c000000, 0x1c000000); // mapping uart
}

void vm_setup(size_t kernel_vaddr, size_t kernel_paddr, size_t kernel_size)
{
    vm_init();
    vm_map_devices();

    extern int bootloader_start[];
    size_t bootloader_start_aligned = ALIGN_TO_TABLE((size_t)bootloader_start);
    map_table(bootloader_start_aligned, bootloader_start_aligned);
#ifdef DEBUG_VMM
    log("map %x to %x", bootloader_start_aligned, bootloader_start_aligned);
#endif

    size_t table_paddr = ALIGN_TO_TABLE(kernel_paddr);
    size_t table_vaddr = ALIGN_TO_TABLE(kernel_vaddr);
    const size_t bytes_per_table = VMM_LV0_ENTITY_COUNT * VMM_PAGE_SIZE;
    const size_t tables_per_kernel = align_size((kernel_size + bytes_per_table - 1) / bytes_per_table, 4);
    for (int i = 0; i < tables_per_kernel; i++) {
        map_table(table_paddr, table_paddr);
        map_table(table_paddr, table_vaddr);
#ifdef DEBUG_VMM
        log("map %x to %x", table_paddr, table_paddr);
        log("map %x to %x", table_paddr, table_vaddr);
#endif
        table_paddr += bytes_per_table;
        table_vaddr += bytes_per_table;
    }

    write_ttbr0((size_t)(pdir));
    write_dacr(0x55555555);
    mmu_enable();
}

void vm_setup_secondary_cpu()
{
    write_ttbr0((uint32_t)(pdir));
    write_dacr(0x55555555);
    mmu_enable();
}