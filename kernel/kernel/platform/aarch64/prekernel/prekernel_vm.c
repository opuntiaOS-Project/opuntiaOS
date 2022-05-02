/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libkern/types.h>

static uint64_t* __attribute__((section(".prekernel_data"))) global_page_table_0;
static uint64_t* __attribute__((section(".prekernel_data"))) global_page_table_1;
static uintptr_t __attribute__((section(".prekernel_data"))) paddr = 0x40000000;
static uintptr_t __attribute__((section(".prekernel_data"))) vaddr = 0xfffffff000000000;

static int get_page_size() __attribute__((section(".prekernel_code")));
static int get_page_mask() __attribute__((section(".prekernel_code")));
static void map_table(size_t phyz, size_t virt) __attribute__((section(".prekernel_code")));
void vm_setup() __attribute__((section(".prekernel_code")));

static int get_page_size()
{
    return 0x1000;
}

static int get_page_mask()
{
    return 0xfff;
}

#define VMM_LV0_ENTITY_COUNT (512)
#define VMM_LV1_ENTITY_COUNT (512)
#define VMM_LV2_ENTITY_COUNT (512)
#define VMM_LV3_ENTITY_COUNT (512)
#define VMM_PAGE_SIZE (get_page_size())

#define PAGE_START(vaddr) ((vaddr & ~(uintptr_t)get_page_mask())
#define FRAME(addr) (addr / VMM_PAGE_SIZE)

#define PTABLE_LV_TOP (3)
#define PTABLE_LV0_VADDR_OFFSET (12)
#define PTABLE_LV1_VADDR_OFFSET (21)
#define PTABLE_LV2_VADDR_OFFSET (30)
#define PTABLE_LV3_VADDR_OFFSET (39)

#define VM_VADDR_OFFSET_AT_LEVEL(vaddr, off, ent) ((vaddr >> off) % ent)

static void map_table(size_t phyz, size_t virt)
{
    const size_t page_covers = (1ull << PTABLE_LV2_VADDR_OFFSET);
    phyz &= ~(page_covers - 1);
    virt &= ~(page_covers - 1);

    // Mapping from level2, as needed.
    uint64_t* page_table = global_page_table_0;
    if (virt >= 0xfffffff000000000) {
        page_table = global_page_table_1;
        virt -= 0xfffffff000000000;
    }

    uint64_t pdesc = 0x00000000000701;
    pdesc |= (uintptr_t)phyz;
    page_table[VM_VADDR_OFFSET_AT_LEVEL(virt, PTABLE_LV2_VADDR_OFFSET, VMM_LV2_ENTITY_COUNT)] = pdesc;
}

void vm_setup()
{
    int tg0 = 0b00;
    int tg1 = 0b10;
    int t0sz = 28;
    int t1sz = 28;

    // TODO(aarch64): Kernel size is set to 8mb, since 4mb is enought, let's use address after 4mb.
    global_page_table_0 = (uint64_t*)(paddr + (4 << 20));
    global_page_table_1 = (uint64_t*)((uintptr_t)global_page_table_0 + (1 << 20));

    map_table(paddr, vaddr);
    map_table(paddr, paddr);
    map_table(0x09000000, 0x09000000);

    extern void enable_mmu_el1(uint64_t ttbr0, uint64_t tcr, uint64_t mair, uint64_t ttbr1);
    enable_mmu_el1((uint64_t)global_page_table_0, 0x135003500 | (tg0 << 14) | (tg1 << 30) | (t1sz << 16) | t0sz, 0x04ff, (uint64_t)global_page_table_1);
}
