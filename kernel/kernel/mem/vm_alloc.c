/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <mem/pmm.h>
#include <mem/vm_alloc.h>
#include <mem/vm_pspace.h>
#include <mem/vmm.h>

uintptr_t vm_alloc_pdir_paddr()
{
    return (uintptr_t)pmm_alloc_aligned(PDIR_SIZE, PDIR_SIZE);
}

uintptr_t vm_alloc_ptable_paddr()
{
    return (uintptr_t)pmm_alloc_aligned(PTABLE_SIZE, PTABLE_SIZE);
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

kmemzone_t vm_alloc_mapped_zone(size_t size, size_t alignment)
{
    if (size % VMM_PAGE_SIZE) {
        size += VMM_PAGE_SIZE - (size % VMM_PAGE_SIZE);
    }
    if (alignment % VMM_PAGE_SIZE) {
        alignment += VMM_PAGE_SIZE - (alignment % VMM_PAGE_SIZE);
    }

    // TODO: Currently only sequence allocation is implemented.
    kmemzone_t zone = kmemzone_new_aligned(size, alignment);
    uintptr_t paddr = (uintptr_t)pmm_alloc_aligned(size, alignment);
    vmm_map_pages_lockless(zone.start, paddr, size / VMM_PAGE_SIZE, PAGE_READABLE | PAGE_WRITABLE);
    return zone;
}

int vm_free_mapped_zone(kmemzone_t zone)
{
    page_desc_t* page = vm_pspace_get_page_desc(zone.start);
    pmm_free((void*)page_desc_get_frame(*page), zone.len);
    vmm_unmap_pages_lockless(zone.start, zone.len / VMM_PAGE_SIZE);
    kmemzone_free(zone);
    return 0;
}

pdirectory_t* vm_alloc_pdir()
{
    kmemzone_t zone = vm_alloc_mapped_zone(PDIR_SIZE, PDIR_SIZE);
    return (pdirectory_t*)zone.ptr;
}

void vm_free_pdir(pdirectory_t* pdir)
{
    if (pdir == vmm_get_kernel_pdir()) {
        return;
    }

    kmemzone_t zone;
    zone.start = (uintptr_t)pdir;
    zone.len = PDIR_SIZE;
    vm_free_mapped_zone(zone);
}