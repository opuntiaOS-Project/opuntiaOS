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
#include <platform/generic/vmm/consts.h>
#include <platform/generic/vmm/pde.h>
#include <platform/generic/vmm/pte.h>

#define VMM_TOTAL_PAGES_PER_TABLE VMM_PTE_COUNT
#define VMM_TOTAL_TABLES_PER_DIRECTORY VMM_PDE_COUNT
#define PDIR_SIZE sizeof(pdirectory_t)
#define PTABLE_SIZE sizeof(ptable_t)
#define IS_INDIVIDUAL_PER_DIR(index) (index < VMM_KERNEL_TABLES_START || (index == VMM_OFFSET_IN_DIRECTORY(pspace_zone.start)))

enum PTABLE_LEVELS {
    PTABLE_LV0 = 0,
    PTABLE_LV1 = 1,
    PTABLE_LV2 = 2,
    PTABLE_LV3 = 3,
};
typedef enum PTABLE_LEVELS ptable_lv_t;

typedef struct {
    page_desc_t entities[VMM_PTE_COUNT];
} ptable_t;

typedef struct pdirectory {
    table_desc_t entities[VMM_PDE_COUNT];
} pdirectory_t;

static const size_t pte_entities_at_level[] = {
    [PTABLE_LV0] = VMM_PTE_COUNT,
    [PTABLE_LV1] = VMM_PDE_COUNT,
    [PTABLE_LV2] = 1,
    [PTABLE_LV3] = 1,
};

static const size_t pte_size_at_level[] = {
    [PTABLE_LV0] = VMM_PTE_COUNT * sizeof(pte_t),
    [PTABLE_LV1] = VMM_PDE_COUNT * sizeof(pte_t),
    [PTABLE_LV2] = 0,
    [PTABLE_LV3] = 0,
};

#endif // _KERNEL_MEM_BITS_VM_H