/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <mem/vmm.h>

#define SET_FLAGS(mmu_flags, mf, arch_flags, af) \
    if (TEST_FLAG(mmu_flags, mf)) {              \
        arch_flags |= af;                        \
    }

static inline void clear_arch_flags(ptable_entity_t* entity)
{
    *entity &= ~((1 << (PAGE_DESC_FRAME_OFFSET)) - 1);
}

ptable_entity_t vm_mmu_to_arch_flags(mmu_flags_t mmu_flags, ptable_lv_t lv)
{
    return 0;
}

mmu_flags_t vm_arch_to_mmu_flags(ptable_entity_t* entity, ptable_lv_t lv)
{
    mmu_flags_t mmu_flags = 0;
    return mmu_flags;
}

mmu_pf_info_flags_t vm_arch_parse_pf_info(arch_pf_info_t info)
{
    mmu_pf_info_flags_t res = 0;
    return res;
}

ptable_state_t vm_ptable_entity_state(ptable_entity_t* entity, ptable_lv_t lv)
{
    ptable_entity_t arch_flags = *entity;
    return PTABLE_ENTITY_INVALID;
}

void vm_ptable_entity_set_default_flags(ptable_entity_t* entity, ptable_lv_t lv)
{
    *entity = 0;
}

void vm_ptable_entity_allocated(ptable_entity_t* entity, ptable_lv_t lv)
{
    *entity = 0;
}

void vm_ptable_entity_invalidate(ptable_entity_t* entity, ptable_lv_t lv)
{
    *entity = 0;
}

void vm_ptable_entity_set_mmu_flags(ptable_entity_t* entity, ptable_lv_t lv, mmu_flags_t mmu_flags)
{
    mmu_flags_t old_mmu_flags = vm_arch_to_mmu_flags(entity, lv);
    old_mmu_flags |= mmu_flags;
    ptable_entity_t arch_flags = vm_mmu_to_arch_flags(old_mmu_flags, lv);
    clear_arch_flags(entity);
    *entity |= arch_flags;
}

void vm_ptable_entity_rm_mmu_flags(ptable_entity_t* entity, ptable_lv_t lv, mmu_flags_t mmu_flags)
{
    mmu_flags_t old_mmu_flags = vm_arch_to_mmu_flags(entity, lv);
    old_mmu_flags &= ~mmu_flags;
    ptable_entity_t arch_flags = vm_mmu_to_arch_flags(old_mmu_flags, lv);
    clear_arch_flags(entity);
    *entity |= arch_flags;
}

void vm_ptable_entity_set_frame(ptable_entity_t* entity, ptable_lv_t lv, uintptr_t frame)
{
    // On x86 frame offset are equal for both levels.
    *entity &= ((1 << (PAGE_DESC_FRAME_OFFSET)) - 1);
    frame >>= PAGE_DESC_FRAME_OFFSET;
    *entity |= (frame << PAGE_DESC_FRAME_OFFSET);
}

uintptr_t vm_ptable_entity_get_frame(ptable_entity_t* entity, ptable_lv_t lv)
{
    // On x86 frame offset are equal for both levels.
    return ((*entity >> PAGE_DESC_FRAME_OFFSET) << PAGE_DESC_FRAME_OFFSET);
}

bool vm_ptable_entity_is_present(ptable_entity_t* entity, ptable_lv_t lv)
{
    if (!entity) {
        return false;
    }
    return vm_ptable_entity_state(entity, lv) == PTABLE_ENTITY_PRESENT;
}

bool vm_ptable_entity_is_only_allocated(ptable_entity_t* entity, ptable_lv_t lv)
{
    if (!entity) {
        return false;
    }
    return vm_ptable_entity_state(entity, lv) == PTABLE_ENTITY_ALLOC;
}
