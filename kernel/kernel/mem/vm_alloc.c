/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libkern/bits/errno.h>
#include <mem/pmm.h>
#include <mem/vm_alloc.h>
#include <mem/vm_pspace.h>
#include <mem/vmm.h>

uintptr_t vm_alloc_pdir_paddr()
{
    return (uintptr_t)pmm_alloc_aligned(PTABLE_SIZE(PTABLE_LV_TOP), PTABLE_SIZE(PTABLE_LV_TOP));
}

uintptr_t vm_alloc_ptable_paddr(ptable_lv_t lv)
{
    return (uintptr_t)pmm_alloc_aligned(PTABLE_SIZE(lv), PTABLE_SIZE(lv));
}

uintptr_t vm_alloc_ptables_to_cover_page()
{
    return (uintptr_t)pmm_alloc_aligned(VMM_PAGE_SIZE, VMM_PAGE_SIZE);
}

void vm_free_ptables_to_cover_page(uintptr_t addr)
{
    pmm_free((void*)addr, VMM_PAGE_SIZE);
}

uintptr_t vm_alloc_page_paddr()
{
    return (uintptr_t)pmm_alloc_aligned(VMM_PAGE_SIZE, VMM_PAGE_SIZE);
}

void vm_free_page_paddr(uintptr_t addr)
{
    pmm_free((void*)addr, VMM_PAGE_SIZE);
}

int vm_alloc_mapped_zone(size_t size, size_t alignment, kmemzone_t* kmemzone)
{
    if (size % VMM_PAGE_SIZE) {
        size += VMM_PAGE_SIZE - (size % VMM_PAGE_SIZE);
    }
    if (alignment % VMM_PAGE_SIZE) {
        alignment += VMM_PAGE_SIZE - (alignment % VMM_PAGE_SIZE);
    }

    // Currntly can map only consequent paddrs, might be fixed.
    uintptr_t paddr = (uintptr_t)pmm_alloc_aligned(size, alignment);
    if (!paddr) {
        return -ENOMEM;
    }

    kmemzone_t zone = kmemzone_new_aligned(size, alignment);
    vmm_map_pages_locked(zone.start, paddr, size / VMM_PAGE_SIZE, MMU_FLAG_PERM_READ | MMU_FLAG_PERM_WRITE);
    *kmemzone = zone;
    return 0;
}

int vm_free_mapped_zone(kmemzone_t zone)
{
    ptable_entity_t* page_desc = vm_get_entity(zone.start, PTABLE_LV0);
    if (!vm_ptable_entity_is_present(page_desc, PTABLE_LV0)) {
        return -1;
    }
    pmm_free((void*)vm_ptable_entity_get_frame(page_desc, PTABLE_LV0), zone.len);
    vmm_unmap_pages_locked(zone.start, zone.len / VMM_PAGE_SIZE);
    kmemzone_free(zone);
    return 0;
}

ptable_t* vm_alloc_ptable_lv_top()
{
    kmemzone_t zone;
    int err = vm_alloc_mapped_zone(ptable_size_at_level[PTABLE_LV_TOP], ptable_size_at_level[PTABLE_LV_TOP], &zone);
    if (err) {
        return NULL;
    }
    return (ptable_t*)zone.ptr;
}

void vm_free_ptable_lv_top(ptable_t* pdir)
{
    if (pdir == vmm_get_kernel_pdir()) {
        return;
    }

    kmemzone_t zone;
    zone.start = (uintptr_t)pdir;
    zone.len = PTABLE_SIZE(PTABLE_LV_TOP);
    vm_free_mapped_zone(zone);
}