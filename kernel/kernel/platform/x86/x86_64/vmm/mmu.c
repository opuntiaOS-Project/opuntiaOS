/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libkern/log.h>
#include <libkern/types.h>
#include <mem/vmm.h>

#define FRAME_OFFSET (12)

#define SET_FLAGS(mmu_flags, mf, arch_flags, af) \
    if (TEST_FLAG(mmu_flags, mf)) {              \
        arch_flags |= af;                        \
    }

#define SET_OP(mmu_flags, mf, op)   \
    if (TEST_FLAG(mmu_flags, mf)) { \
        op;                         \
    }

#define SET_OP_NEG(mmu_flags, mf, op) \
    if (!TEST_FLAG(mmu_flags, mf)) {  \
        op;                           \
    }

static inline void clear_arch_flags(ptable_entity_t* entity)
{
    *entity &= ~((1ull << (FRAME_OFFSET)) - 1);
    *entity &= ((1ull << (52)) - 1);
}

static ptable_entity_t terminating_page_common_mmu_to_arch_flags(mmu_flags_t mmu_flags)
{
    ptable_entity_t arch_flags = 0;

    SET_OP(mmu_flags, MMU_FLAG_PERM_WRITE, arch_flags |= (1 << 1));
    SET_OP(mmu_flags, MMU_FLAG_NONPRIV, arch_flags |= (1 << 2));
    SET_OP(mmu_flags, MMU_FLAG_UNCACHED, arch_flags |= (1 << 4));

    return arch_flags;
}

ptable_entity_t vm_mmu_to_arch_flags(mmu_flags_t mmu_flags, ptable_lv_t lv)
{
    ptable_entity_t arch_flags = 0;
    vm_ptable_entity_set_default_flags(&arch_flags, lv);

    switch (lv) {
    case PTABLE_LV0:
        SET_OP(mmu_flags, MMU_FLAG_PERM_READ, arch_flags |= 0x1);
        return arch_flags | terminating_page_common_mmu_to_arch_flags(mmu_flags);

    case PTABLE_LV1:
    case PTABLE_LV2:
        if (TEST_FLAG(mmu_flags, MMU_FLAG_HUGE_PAGE)) {
            SET_OP(mmu_flags, MMU_FLAG_PERM_READ, arch_flags |= 0x1);
            arch_flags |= (1 << 7); // Setting huge page.
            return arch_flags | terminating_page_common_mmu_to_arch_flags(mmu_flags);
        }
        // fallthrough
    case PTABLE_LV3:
        // If the entry is not a terminating one, we allow the maximum
        // of permissions (e.g user/writable/cachable...), so only the
        // leaf entries control the access.
        SET_OP(mmu_flags, MMU_FLAG_PERM_READ, arch_flags |= 0x7);
    }

    return arch_flags;
}

static mmu_flags_t terminating_page_common_arch_to_mmu_flags(ptable_entity_t* entity)
{
    ptable_entity_t arch_flags = *entity;
    mmu_flags_t mmu_flags = 0;

    SET_FLAGS(arch_flags, (1 << 0), mmu_flags, MMU_FLAG_PERM_READ);
    SET_FLAGS(arch_flags, (1 << 1), mmu_flags, MMU_FLAG_PERM_WRITE);
    SET_FLAGS(arch_flags, (1 << 2), mmu_flags, MMU_FLAG_NONPRIV);
    SET_FLAGS(arch_flags, (1 << 4), mmu_flags, MMU_FLAG_UNCACHED);

    return mmu_flags;
}

mmu_flags_t vm_arch_to_mmu_flags(ptable_entity_t* entity, ptable_lv_t lv)
{
    ptable_entity_t arch_flags = *entity;
    mmu_flags_t mmu_flags = 0;

    switch (lv) {
    case PTABLE_LV0:
        return mmu_flags | terminating_page_common_arch_to_mmu_flags(entity);

    case PTABLE_LV1:
    case PTABLE_LV2:
        if (TEST_FLAG(arch_flags, (1 << 7))) {
            mmu_flags |= MMU_FLAG_HUGE_PAGE;
            return mmu_flags | terminating_page_common_arch_to_mmu_flags(entity);
        }
        // fallthrough
    case PTABLE_LV3:
        SET_FLAGS(arch_flags, (1 << 0), mmu_flags, MMU_FLAG_PERM_READ);
        mmu_flags |= MMU_FLAG_PERM_WRITE | MMU_FLAG_NONPRIV;
        return mmu_flags;
    }

    return mmu_flags;
}

mmu_pf_info_flags_t vm_arch_parse_pf_info(arch_pf_info_t info)
{
    mmu_pf_info_flags_t res = 0;
    if ((info & 0b100) == 0b100) {
        res |= MMU_PF_INFO_ON_NONPRIV_ACCESS;
    }
    if ((info & 0b010) == 0b010) {
        res |= MMU_PF_INFO_ON_WRITE;
    }
    if ((info & 0b001) == 0) {
        res |= MMU_PF_INFO_ON_NOT_PRESENT;
    }
    return res;
}

ptable_state_t vm_ptable_entity_state(ptable_entity_t* entity, ptable_lv_t lv)
{
    ptable_entity_t arch_flags = *entity;

    switch (lv) {
    case PTABLE_LV0:
    case PTABLE_LV1:
    case PTABLE_LV2:
    case PTABLE_LV3:
        if (TEST_FLAG(arch_flags, (1 << 0))) {
            return PTABLE_ENTITY_PRESENT;
        } else {
            return PTABLE_ENTITY_INVALID;
        }
    }

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
    // On x86 frame offset are equal for all levels.
    *entity &= ((1 << (FRAME_OFFSET)) - 1);
    frame >>= FRAME_OFFSET;
    *entity |= (frame << FRAME_OFFSET);
}

uintptr_t vm_ptable_entity_get_frame(ptable_entity_t* entity, ptable_lv_t lv)
{
    // On x86 frame offset are equal for all levels.
    return ((*entity >> FRAME_OFFSET) << FRAME_OFFSET);
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
    // This is for 32bit systems only.
    return false;
}