/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_MEM_BITS_MMU_H
#define _KERNEL_MEM_BITS_MMU_H

#include <libkern/types.h>

enum MMU_FLAGS {
    MMU_FLAG_PERM_WRITE = (1 << 0),
    MMU_FLAG_PERM_READ = (1 << 1),
    MMU_FLAG_PERM_EXEC = (1 << 2),
    MMU_FLAG_UNCACHED = (1 << 3),
    MMU_FLAG_NONPRIV = (1 << 4),
    MMU_FLAG_INVALID = (1 << 5),
    MMU_FLAG_COW = (1 << 6), // TODO: Remove this flag.
    MMU_FLAG_DEVICE = MMU_FLAG_PERM_READ | MMU_FLAG_PERM_WRITE | MMU_FLAG_UNCACHED,
};
typedef uint32_t mmu_flags_t;

enum MMU_PF_INFO_FLAGS {
    MMU_PF_INFO_ON_NOT_PRESENT = (1 << 0),
    MMU_PF_INFO_ON_WRITE = (1 << 1),
    MMU_PF_INFO_ON_NONPRIV_ACCESS = (1 << 2),
    MMU_PF_INFO_SECURITY_VIOLATION = (1 << 3),
};
typedef uint32_t mmu_pf_info_flags_t;

enum PTABLE_LEVELS {
    PTABLE_LV0 = 0,
    PTABLE_LV1 = 1,
    PTABLE_LV2 = 2,
    PTABLE_LV3 = 3,
};
typedef enum PTABLE_LEVELS ptable_lv_t;

enum PTABLE_ENTITY_STATES {
    PTABLE_ENTITY_PRESENT,
    PTABLE_ENTITY_INVALID,
    PTABLE_ENTITY_ALLOC, // For arm32 pspace, other targets never return this state.
};
typedef uint32_t ptable_state_t;

#endif // _KERNEL_MEM_BITS_MMU_H