/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_MEM_BITS_VM_H
#define _KERNEL_MEM_BITS_VM_H

#include <libkern/types.h>
#include <mem/bits/mmu.h>
#include <platform/generic/vmm/consts.h>
#include <platform/generic/vmm/mmu.h>

#define PTABLE_ENTITY_COUNT(lv) (ptable_entity_count_at_level[lv])
#define PTABLE_SIZE(lv) (ptable_size_at_level[lv])
#define IS_INDIVIDUAL_PER_DIR(index) (index < VMM_KERNEL_TABLES_START || (index == VMM_OFFSET_IN_DIRECTORY(pspace_zone.start)))

typedef struct {
    ptable_entity_t entities[1];
} ptable_t;

static const size_t ptable_entity_count_at_level[] = {
    [PTABLE_LV0] = VMM_PTE_COUNT,
    [PTABLE_LV1] = VMM_PDE_COUNT,
    [PTABLE_LV2] = 1,
    [PTABLE_LV3] = 1,
};

static const size_t ptable_size_at_level[] = {
    [PTABLE_LV0] = VMM_PTE_COUNT * sizeof(ptable_entity_t),
    [PTABLE_LV1] = VMM_PDE_COUNT * sizeof(ptable_entity_t),
    [PTABLE_LV2] = 0,
    [PTABLE_LV3] = 0,
};

static const size_t ptable_entity_vaddr_offset_at_level[] = {
    [PTABLE_LV0] = PTABLE_LV0_VADDR_OFFSET,
    [PTABLE_LV1] = PTABLE_LV1_VADDR_OFFSET,
    [PTABLE_LV2] = 32,
    [PTABLE_LV3] = 32,
};

#define VM_VADDR_OFFSET_AT_LEVEL(vaddr, lv) ((vaddr >> ptable_entity_vaddr_offset_at_level[lv]) % ptable_entity_count_at_level[lv])

static inline ptable_lv_t lower_level(ptable_lv_t lv)
{
    ASSERT(lv != PTABLE_LV0);
    return lv - 1;
}

static inline ptable_lv_t upper_level(ptable_lv_t lv)
{
    ASSERT(lv != PTABLE_LV_TOP);
    return lv + 1;
}

#endif // _KERNEL_MEM_BITS_VM_H