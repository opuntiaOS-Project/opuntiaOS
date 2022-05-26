/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libkern/types.h>
#include <mem/boot.h>

static uint64_t* global_page_table_0;
static uint64_t* global_page_table_1;

// This is setup for 4KB pages
static const int tg0 = 0b00;
static const int tg1 = 0b10;
static const int t0sz = 25;
static const int t1sz = 25;
static const uint64_t kernel_base = 0xffffffffffffffff - ((1ull << (64 - t1sz)) - 1);

static uint64_t* new_ptable(boot_args_t* args);
static void map4kb_1gb(boot_args_t* args, size_t phyz, size_t virt);
static void map4kb_2mb(boot_args_t* args, size_t phyz, size_t virt);
void prekernel_vm_setup(boot_args_t* args);
void* prekernel_memset(void* dest, uint8_t fll, size_t nbytes);

static void* prekernel_translate_addr(boot_args_t* args, void* addr);
boot_args_t* prekernel_move_args(boot_args_t* args);

#define ROUND_CEIL(a, b) (((a) + ((b)-1)) & ~((b)-1))
#define ROUND_FLOOR(a, b) ((a) & ~((b)-1))

#define VMM_LV0_ENTITY_COUNT (512)
#define VMM_LV1_ENTITY_COUNT (512)
#define VMM_LV2_ENTITY_COUNT (512)
#define VMM_LV3_ENTITY_COUNT (512)

#define PTABLE_LV_TOP (2)
#define PTABLE_LV0_VADDR_OFFSET (12)
#define PTABLE_LV1_VADDR_OFFSET (21)
#define PTABLE_LV2_VADDR_OFFSET (30)
#define PTABLE_LV3_VADDR_OFFSET (39)

#define VM_VADDR_OFFSET_AT_LEVEL(vaddr, off, ent) ((vaddr >> off) % ent)

uintptr_t next_alloc_addr = 0x0;
static uint64_t* new_ptable(boot_args_t* args)
{
    uint64_t* res = (uint64_t*)next_alloc_addr;
    prekernel_memset(res, 0, 4 << 10);
    next_alloc_addr += 4 << 10; // 4KB
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

void* prekernel_memset(void* dest, uint8_t fll, size_t nbytes)
{
    for (int i = 0; i < nbytes; ++i) {
        *((uint8_t*)dest + i) = fll;
    }
    return dest;
}

void prekernel_vm_setup(boot_args_t* args)
{
    // This implementation is a stub, supporting only 4kb pages for now.
    // We should support 16kb pages for sure on modern Apls.
    global_page_table_0 = (uint64_t*)(ROUND_CEIL(args->paddr + args->kernel_size, 16 << 10));
    prekernel_memset(global_page_table_0, 0, 4 << 10);

    global_page_table_1 = (uint64_t*)((uintptr_t)global_page_table_0 + (16 << 10));
    prekernel_memset(global_page_table_1, 0, 4 << 10);

    next_alloc_addr = (uintptr_t)global_page_table_1 + (16 << 10);
    args->kernel_size += 1 << 20;

    map4kb_2mb(args, args->paddr, kernel_base);
    map4kb_2mb(args, args->paddr + (2 << 20), kernel_base + (2 << 20));
    map4kb_2mb(args, args->paddr, args->vaddr);
    map4kb_2mb(args, args->paddr, args->paddr);
    map4kb_2mb(args, args->paddr + (2 << 20), args->paddr + (2 << 20));

    if (args->fb_boot_desc.vaddr == 0) {
        map4kb_1gb(args, 0x0, 0x0);
    } else {
        uint64_t paddr = args->fb_boot_desc.paddr;
        const size_t page_covers = (1ull << PTABLE_LV2_VADDR_OFFSET);
        paddr &= ~(page_covers - 1);

        map4kb_1gb(args, paddr, 0xfc0000000ULL);
        args->fb_boot_desc.vaddr = 0xfc0000000ULL + (args->fb_boot_desc.paddr - paddr);
    }

    extern void enable_mmu_el1(uint64_t ttbr0, uint64_t tcr, uint64_t mair, uint64_t ttbr1);
    enable_mmu_el1((uint64_t)global_page_table_0, 0x135003500 | (tg0 << 14) | (tg1 << 30) | (t1sz << 16) | t0sz, 0x04ff, (uint64_t)global_page_table_1);
}

static void* prekernel_translate_addr(boot_args_t* args, void* addr)
{
    if (!addr) {
        return NULL;
    }
    return (void*)((uintptr_t)addr - args->vaddr + kernel_base);
}

boot_args_t* prekernel_move_args(boot_args_t* args)
{
    boot_args_t* newargs = (boot_args_t*)((uintptr_t)args - args->vaddr + kernel_base);
    newargs->memory_map = prekernel_translate_addr(newargs, newargs->memory_map);
    newargs->devtree = prekernel_translate_addr(newargs, newargs->devtree);
    newargs->vaddr = kernel_base;
    return newargs;
}