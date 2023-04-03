/*
 * Copyright (C) 2020-2023 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_RISCV64_VMM_MMU_H
#define _KERNEL_PLATFORM_RISCV64_VMM_MMU_H

#include <libkern/types.h>
#include <mem/bits/mmu.h>

// riscv64 uses one table to manage virtual space.
#undef DOUBLE_TABLE_PAGING

typedef uint64_t ptable_entity_t;
typedef uint64_t arch_pf_info_t;

ptable_entity_t vm_mmu_to_arch_flags(mmu_flags_t mmu_flags, ptable_lv_t lv);
mmu_flags_t vm_arch_to_mmu_flags(ptable_entity_t* entity, ptable_lv_t lv);
mmu_pf_info_flags_t vm_arch_parse_pf_info(arch_pf_info_t info);

void vm_ptable_entity_set_default_flags(ptable_entity_t* entity, ptable_lv_t lv);
void vm_ptable_entity_allocated(ptable_entity_t* entity, ptable_lv_t lv);
void vm_ptable_entity_invalidate(ptable_entity_t* entity, ptable_lv_t lv);

void vm_ptable_entity_set_mmu_flags(ptable_entity_t* entity, ptable_lv_t lv, mmu_flags_t mmu_flags);
void vm_ptable_entity_rm_mmu_flags(ptable_entity_t* entity, ptable_lv_t lv, mmu_flags_t mmu_flags);
void vm_ptable_entity_set_frame(ptable_entity_t* entity, ptable_lv_t lv, uintptr_t frame);
uintptr_t vm_ptable_entity_get_frame(ptable_entity_t* entity, ptable_lv_t lv);

ptable_state_t vm_ptable_entity_state(ptable_entity_t* entity, ptable_lv_t lv);
bool vm_ptable_entity_is_present(ptable_entity_t* entity, ptable_lv_t lv);
bool vm_ptable_entity_is_only_allocated(ptable_entity_t* entity, ptable_lv_t lv);

#endif // _KERNEL_PLATFORM_RISCV64_VMM_MMU_H