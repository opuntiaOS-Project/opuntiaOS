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
#include <platform/aarch64/vmm/pde.h>
#include <platform/aarch64/vmm/pte.h>

#define SET_FLAGS(mmu_flags, mf, arch_flags, af) \
    if (TEST_FLAG(mmu_flags, mf)) {              \
        arch_flags |= af;                        \
    }

#define SET_OP(mmu_flags, mf, op)   \
    if (TEST_FLAG(mmu_flags, mf)) { \
        op;                         \
    }

static inline void clear_arch_flags(ptable_entity_t* entity, ptable_lv_t lv)
{
    // switch (lv) {
    // case PTABLE_LV0:
    //     *entity &= ~((1 << (PAGE_DESC_FRAME_OFFSET)) - 1);
    //     return;

    // case PTABLE_LV1:
    //     *entity &= ~((1 << (TABLE_DESC_FRAME_OFFSET)) - 1);
    //     return;

    // case PTABLE_LV2:
    // case PTABLE_LV3:
    //     ASSERT(false);
    // }
}

ptable_entity_t vm_mmu_to_arch_flags(mmu_flags_t mmu_flags, ptable_lv_t lv)
{
    ptable_entity_t arch_flags = 0;
    // page_desc_t* arch_page_flags = (page_desc_t*)&arch_flags;
    // table_desc_t* arch_table_flags = (table_desc_t*)&arch_flags;
    // vm_ptable_entity_set_default_flags(&arch_flags, lv);

    // switch (lv) {
    // case PTABLE_LV0:
    //     SET_OP(mmu_flags, MMU_FLAG_PERM_READ, arch_page_flags->one = 1);
    //     // Note: access-order to ap1 is important to set MMU flags correctly.
    //     SET_OP(mmu_flags, MMU_FLAG_NONPRIV, arch_page_flags->ap1 = 0b10);
    //     SET_OP(mmu_flags, MMU_FLAG_PERM_WRITE, arch_page_flags->ap1 |= 0b01);
    //     SET_OP(mmu_flags, MMU_FLAG_UNCACHED, arch_page_flags->c = 0);
    //     return arch_flags;

    // case PTABLE_LV1:
    //     SET_OP(mmu_flags, MMU_FLAG_PERM_READ, arch_table_flags->valid = 1);
    //     SET_OP(mmu_flags, MMU_FLAG_COW, arch_table_flags->imp = 1);
    //     return arch_flags;

    // case PTABLE_LV2:
    // case PTABLE_LV3:
    //     ASSERT(false);
    // }

    return arch_flags;
}

mmu_flags_t vm_arch_to_mmu_flags(ptable_entity_t* entity, ptable_lv_t lv)
{
    // page_desc_t* arch_page_flags = (page_desc_t*)entity;
    // table_desc_t* arch_table_flags = (table_desc_t*)entity;

    mmu_flags_t mmu_flags = 0;
    // switch (lv) {
    // case PTABLE_LV0:
    //     if (arch_page_flags->one != 0) {
    //         mmu_flags |= MMU_FLAG_PERM_READ;
    //     }

    //     if (arch_page_flags->c == 0) {
    //         mmu_flags |= MMU_FLAG_UNCACHED;
    //     }

    //     if (arch_page_flags->ap1 == 0b11) {
    //         mmu_flags |= MMU_FLAG_NONPRIV | MMU_FLAG_PERM_WRITE;
    //     } else if (arch_page_flags->ap1 == 0b10) {
    //         mmu_flags |= MMU_FLAG_NONPRIV;
    //     } else {
    //         mmu_flags |= MMU_FLAG_PERM_WRITE;
    //     }

    //     return mmu_flags;

    // case PTABLE_LV1:
    //     mmu_flags |= MMU_FLAG_PERM_READ | MMU_FLAG_PERM_WRITE | MMU_FLAG_NONPRIV;
    //     if (arch_table_flags->imp) {
    //         mmu_flags |= MMU_FLAG_COW;
    //     }
    //     return mmu_flags;

    // case PTABLE_LV2:
    // case PTABLE_LV3:
    //     ASSERT(false);
    // }

    return mmu_flags;
}

mmu_pf_info_flags_t vm_arch_parse_pf_info(arch_pf_info_t info)
{
    mmu_pf_info_flags_t res = 0;

    // // pl0 bit is set at aarch32/interrupt_handlers.c
    // if (((info >> 31) & 0x1) == 0) {
    //     res |= MMU_PF_INFO_ON_NONPRIV_ACCESS;
    // }
    // if (((info >> 11) & 0x1) == 0x1) {
    //     res |= MMU_PF_INFO_ON_WRITE;
    // }
    // if ((info & 0b1101) == 0b0101) {
    //     res |= MMU_PF_INFO_ON_NOT_PRESENT;
    // }
    // if ((info & 0b1111) == 0b1111) {
    //     res |= MMU_PF_INFO_SECURITY_VIOLATION;
    // }
    return res;
}

ptable_state_t vm_ptable_entity_state(ptable_entity_t* entity, ptable_lv_t lv)
{
    // ptable_entity_t arch_flags = *entity;
    // page_desc_t* arch_page_flags = (page_desc_t*)entity;
    // table_desc_t* arch_table_flags = (table_desc_t*)entity;

    // switch (lv) {
    // case PTABLE_LV0:
    //     if (arch_page_flags->one != 0) {
    //         return PTABLE_ENTITY_PRESENT;
    //     } else {
    //         return PTABLE_ENTITY_INVALID;
    //     }

    // case PTABLE_LV1:
    //     if ((arch_table_flags->data & 0b0111111111) == (0b0001111000)) {
    //         return PTABLE_ENTITY_ALLOC;
    //     } else if (arch_table_flags->valid) {
    //         return PTABLE_ENTITY_PRESENT;
    //     } else {
    //         return PTABLE_ENTITY_INVALID;
    //     }

    // case PTABLE_LV2:
    // case PTABLE_LV3:
    //     ASSERT(false);
    // }

    return PTABLE_ENTITY_INVALID;
}

void vm_ptable_entity_set_default_flags(ptable_entity_t* entity, ptable_lv_t lv)
{
    // page_desc_t* arch_page_flags = (page_desc_t*)entity;
    // table_desc_t* arch_table_flags = (table_desc_t*)entity;

    // switch (lv) {
    // case PTABLE_LV0:
    //     arch_page_flags->data = 0;
    //     arch_page_flags->one = 1;
    //     arch_page_flags->ap1 = 0b01; // Kernel -- R/W and User -- No access
    //     arch_page_flags->ap2 = 0;
    //     arch_page_flags->c = 1;
    //     arch_page_flags->s = 1;
    //     arch_page_flags->b = 1;
    //     arch_page_flags->tex = 0b001;
    //     return;

    // case PTABLE_LV1:
    //     arch_table_flags->data = 0;
    //     arch_table_flags->valid = 0;
    //     arch_table_flags->zero1 = 0;
    //     arch_table_flags->zero2 = 0;
    //     arch_table_flags->zero3 = 0;
    //     arch_table_flags->imp = 0;
    //     arch_table_flags->domain = 0b0011;
    //     return;

    // case PTABLE_LV2:
    // case PTABLE_LV3:
    //     ASSERT(false);
    // }
}

void vm_ptable_entity_allocated(ptable_entity_t* entity, ptable_lv_t lv)
{
    // table_desc_t* arch_table_flags = (table_desc_t*)entity;

    // switch (lv) {
    // case PTABLE_LV1:
    //     arch_table_flags->data = 0;
    //     arch_table_flags->valid = 0;
    //     arch_table_flags->zero1 = 0;
    //     arch_table_flags->zero2 = 0;
    //     arch_table_flags->zero3 = 1;
    //     arch_table_flags->ns = 1;
    //     arch_table_flags->imp = 0;
    //     arch_table_flags->domain = 0b0011;
    //     return;

    // case PTABLE_LV0:
    // case PTABLE_LV2:
    // case PTABLE_LV3:
    //     ASSERT(false);
    // }
}

void vm_ptable_entity_invalidate(ptable_entity_t* entity, ptable_lv_t lv)
{
    *entity = 0;
}

void vm_ptable_entity_set_mmu_flags(ptable_entity_t* entity, ptable_lv_t lv, mmu_flags_t mmu_flags)
{
    // mmu_flags_t old_mmu_flags = vm_arch_to_mmu_flags(entity, lv);
    // old_mmu_flags |= mmu_flags;
    // ptable_entity_t arch_flags = vm_mmu_to_arch_flags(old_mmu_flags, lv);
    // clear_arch_flags(entity, lv);
    // *entity |= arch_flags;
}

void vm_ptable_entity_rm_mmu_flags(ptable_entity_t* entity, ptable_lv_t lv, mmu_flags_t mmu_flags)
{
    // mmu_flags_t old_mmu_flags = vm_arch_to_mmu_flags(entity, lv);
    // old_mmu_flags &= ~mmu_flags;
    // ptable_entity_t arch_flags = vm_mmu_to_arch_flags(old_mmu_flags, lv);
    // clear_arch_flags(entity, lv);
    // *entity |= arch_flags;
}

void vm_ptable_entity_set_frame(ptable_entity_t* entity, ptable_lv_t lv, uintptr_t frame)
{
    // switch (lv) {
    // case PTABLE_LV0:
    //     *entity &= ((1 << (PAGE_DESC_FRAME_OFFSET)) - 1);
    //     frame >>= PAGE_DESC_FRAME_OFFSET;
    //     *entity |= (frame << PAGE_DESC_FRAME_OFFSET);
    //     return;

    // case PTABLE_LV1:
    //     *entity &= ((1 << (TABLE_DESC_FRAME_OFFSET)) - 1);
    //     frame >>= TABLE_DESC_FRAME_OFFSET;
    //     *entity |= (frame << TABLE_DESC_FRAME_OFFSET);
    //     return;

    // case PTABLE_LV2:
    // case PTABLE_LV3:
    //     ASSERT(false);
    // }
}

uintptr_t vm_ptable_entity_get_frame(ptable_entity_t* entity, ptable_lv_t lv)
{
    // switch (lv) {
    // case PTABLE_LV0:
    //     return ((*entity >> PAGE_DESC_FRAME_OFFSET) << PAGE_DESC_FRAME_OFFSET);

    // case PTABLE_LV1:
    //     return ((*entity >> TABLE_DESC_FRAME_OFFSET) << TABLE_DESC_FRAME_OFFSET);

    // case PTABLE_LV2:
    // case PTABLE_LV3:
    //     ASSERT(false);
    // }

    return 0;
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
    // TODO(aarch64): Mark this somehow.
    // This is for 32bit systems only.
    return false;
}
