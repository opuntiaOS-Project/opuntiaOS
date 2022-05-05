/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libkern/bits/errno.h>
#include <libkern/libkern.h>
#include <libkern/lock.h>
#include <libkern/log.h>
#include <mem/kmemzone.h>
#include <mem/memzone.h>
#include <mem/vm_alloc.h>
#include <mem/vm_pspace.h>
#include <mem/vmm.h>
#include <platform/generic/cpu.h>
#include <platform/generic/system.h>

/**
 * VM system for 32bit machines is designed to support 2-level address
 * translation mechanism.
 * Pspace for 32bit arches is located right after the kernel and has
 * all ptables at the 2nd level mapped to this area. So, the size of
 * pspace is 4mb.
 *
 * TODO: Add detailed notes about the VM system design.
 */

static kmemzone_t pspace_zone;
uintptr_t vm_pspace_paddr_zone_offset = 0x0;

void* paddr_to_vaddr(uintptr_t paddr)
{
    return (void*)(vm_pspace_paddr_zone_offset + (uintptr_t)paddr);
}

void vm_pspace_init(boot_args_t* args)
{
}

/**
 * @brief Returns pointer to an active ptable at lv level
 *
 * @param vaddr The virtual address to retrieve a ptable for
 * @param lv The level of the table that defines the target entity
 * @return Pointer to the ptable. Might return NULL if not found
 */
ptable_t* vm_get_table_impl(ptable_t* cur, uintptr_t vaddr, ptable_lv_t lv, ptable_lv_t needlv)
{
    if (lv == needlv) {
        return cur;
    }

    ptable_entity_t* ptable_desc = vm_lookup(cur, lv, vaddr);
    if (!vm_ptable_entity_is_present(ptable_desc, lv)) {
        return NULL;
    }

    ptable_t* next_ptable = paddr_to_vaddr(vm_ptable_entity_get_frame(ptable_desc, lv));
    return vm_get_table_impl(next_ptable, vaddr, lower_level(lv), needlv);
}

ptable_t* vm_get_table(uintptr_t vaddr, ptable_lv_t lv)
{
    if (!THIS_CPU->active_address_space) {
        return NULL;
    }

    ptable_t* active_pdir = THIS_CPU->active_address_space->pdir0;
    if (IS_KERNEL_VADDR(vaddr)) {
        active_pdir = THIS_CPU->active_address_space->pdir1;
    }
    ASSERT(active_pdir);

    return vm_get_table_impl(active_pdir, vaddr, PTABLE_LV_TOP, lv);
}

/**
 * @brief Returns entity of an active ptable at lv level
 *
 * @param vaddr The virtual address to retrieve an entity for
 * @param lv The level of the table that defines the target entity
 * @return Pointer to the ptable entity. Might return NULL if not found
 */
ptable_entity_t* vm_get_entity(uintptr_t vaddr, ptable_lv_t lv)
{
    ptable_t* ptable = vm_get_table(vaddr, lv);
    if (!ptable) {
        return NULL;
    }

    return vm_lookup(ptable, lv, vaddr);
}

static int vm_pspace_free_page_locked(uintptr_t vaddr, ptable_entity_t* page, dynamic_array_t* zones)
{
    if (vmm_is_copy_on_write(vaddr)) {
        return -EBUSY;
    }

    if (!vm_ptable_entity_is_present(page, PTABLE_LV0)) {
        return 0;
    }

    ASSERT(false);
    // uintptr_t frame = vm_ptable_entity_get_frame(page, PTABLE_LV0);
    // vm_ptable_entity_invalidate(page, PTABLE_LV0);

    // memzone_t* zone = memzone_find_no_proc(zones, vaddr);
    // if (zone) {
    //     if (zone->type & ZONE_TYPE_DEVICE) {
    //         return 0;
    //     }
    // }

    // vm_free_page_paddr(frame);
    return 0;
}

static int vm_pspace_free_ptable_locked(uintptr_t vaddr)
{
    ASSERT(false);
}

/**
 * @brief Clears address space, including all ptables.
 *
 * @param vm_aspace Address space to be cleared
 * @return Return status
 */
int vm_pspace_free_address_space_locked(vm_address_space_t* vm_aspace)
{
    // vm_address_space_t* prev_aspace = vmm_get_active_address_space();
    // vmm_switch_address_space_locked(vm_aspace);

    // size_t table_coverage = VMM_PAGE_SIZE * PTABLE_ENTITY_COUNT(PTABLE_LV0);
    // for (int i = 0; i < VMM_KERNEL_TABLES_START; i++) {
    //     vm_pspace_free_ptable_locked(table_coverage * i);
    // }

    // if (prev_aspace == vm_aspace) {
    //     vmm_switch_address_space_locked(vmm_get_kernel_address_space());
    // } else {
    //     vmm_switch_address_space_locked(prev_aspace);
    // }
    // vm_pspace_free(vm_aspace->pdir);
    // vm_free_ptable_lv_top(vm_aspace->pdir);
    ASSERT(false);
    return 0;
}