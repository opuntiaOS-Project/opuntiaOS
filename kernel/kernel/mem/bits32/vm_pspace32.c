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
#include <platform/generic/vmm/mapping_table.h>

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

const ptable_lv_t PSPACE_TABLE_LV = PTABLE_LV0;

ptable_t* vm_pspace_get_nth_active_ptable(size_t n, ptable_lv_t lv)
{
    ASSERT(lv == PTABLE_LV0);
    return (ptable_t*)(pspace_zone.start + n * PTABLE_SIZE(lv));
}

ptable_t* vm_pspace_get_vaddr_of_active_ptable(uintptr_t vaddr, ptable_lv_t lv)
{
    ASSERT(lv == PTABLE_LV0);
    return (ptable_t*)vm_pspace_get_nth_active_ptable(VM_VADDR_OFFSET_AT_LEVEL(vaddr, upper_level(lv)), lv);
}

/**
 * @brief Inits pspace
 *
 * @note Called only during the first stage of VM init.
 */
void vm_pspace_init(boot_args_t* args)
{
    pspace_zone = kmemzone_new(4 * MB);

    if (VMM_OFFSET_IN_TABLE(pspace_zone.start) != 0) {
        kpanic("WRONG PSPACE START ADDR");
    }

    /* The code assumes that the length of tables which cover pspace
       is 4KB and that the tables are fit in a single page and are continuous. */
    extern uintptr_t kernel_ptables_start_paddr;
    const size_t ptables_per_page = VMM_PAGE_SIZE / PTABLE_SIZE(PTABLE_LV0);
    uintptr_t kernel_ptabels_vaddr = pspace_zone.start + VMM_KERNEL_TABLES_START * PTABLE_SIZE(PTABLE_LV0);
    uintptr_t kernel_ptabels_paddr = kernel_ptables_start_paddr;
    for (int i = VMM_KERNEL_TABLES_START; i < PTABLE_ENTITY_COUNT(PTABLE_LV_TOP); i += ptables_per_page) {
        ptable_entity_t* ptable_desc = vm_lookup(vmm_get_kernel_address_space()->pdir, PTABLE_LV_TOP, kernel_ptabels_vaddr);
        if (!vm_ptable_entity_is_present(ptable_desc, PTABLE_LV_TOP)) {
            kpanic("vm_pspace_init: pspace should present");
        }

        ptable_t* ptable_vaddr = (ptable_t*)(kernel_ptables_start_paddr + (VMM_OFFSET_IN_DIRECTORY(kernel_ptabels_vaddr) - VMM_KERNEL_TABLES_START) * PTABLE_SIZE(PTABLE_LV0));
        ptable_entity_t* page_desc = vm_lookup(ptable_vaddr, PTABLE_LV0, kernel_ptabels_vaddr);
        vm_ptable_entity_set_default_flags(page_desc, PTABLE_LV0);
        vm_ptable_entity_set_mmu_flags(page_desc, PTABLE_LV0, MMU_FLAG_PERM_READ | MMU_FLAG_PERM_WRITE | MMU_FLAG_UNCACHED);
        vm_ptable_entity_set_frame(page_desc, PTABLE_LV0, kernel_ptabels_paddr);
        kernel_ptabels_vaddr += VMM_PAGE_SIZE;
        kernel_ptabels_paddr += VMM_PAGE_SIZE;
    }
}

/**
 * @brief Generates a new psapce for pdir
 *
 * @note The length of pspace table is 4KB long.
 */
void vm_pspace_gen(ptable_t* pdir)
{
    const uintptr_t ptables_per_page = VMM_PAGE_SIZE / PTABLE_SIZE(PTABLE_LV0);
    ptable_t* cur_ptable = vm_pspace_get_nth_active_ptable(VMM_OFFSET_IN_DIRECTORY(pspace_zone.start), PTABLE_LV0);
    uintptr_t ptable_paddr = vm_alloc_ptables_to_cover_page();
    ASSERT(ptable_paddr);
    kmemzone_t tmp_zone = kmemzone_new(VMM_PAGE_SIZE);
    ptable_t* new_ptable = (ptable_t*)tmp_zone.start;

    vmm_map_page_locked((uintptr_t)new_ptable, ptable_paddr, MMU_FLAG_PERM_READ | MMU_FLAG_PERM_WRITE);

    /* The code assumes that the length of tables which cover pspace
       is 4KB and that the tables are fit in a single page and are continuous. */
    memcpy(new_ptable, cur_ptable, VMM_PAGE_SIZE);

    ptable_entity_t pspace_page;
    vm_ptable_entity_set_default_flags(&pspace_page, PTABLE_LV0);
    vm_ptable_entity_set_mmu_flags(&pspace_page, PTABLE_LV0, MMU_FLAG_PERM_READ | MMU_FLAG_PERM_WRITE | MMU_FLAG_UNCACHED);
    vm_ptable_entity_set_frame(&pspace_page, PTABLE_LV0, ptable_paddr);

    /* According to prev comment, we can remain overflow here, since we write to the right memory cell */
    new_ptable->entities[VMM_OFFSET_IN_DIRECTORY(pspace_zone.start) / ptables_per_page] = pspace_page;

    size_t table_coverage = VMM_PAGE_SIZE * PTABLE_ENTITY_COUNT(PTABLE_LV0);
    uintptr_t ptable_vaddr_for = pspace_zone.start;
    uintptr_t ptable_paddr_for = ptable_paddr;
    for (int i = 0; i < ptables_per_page; i++, ptable_vaddr_for += table_coverage, ptable_paddr_for += PTABLE_SIZE(PTABLE_LV0)) {
        ptable_entity_t pspace_table;
        vm_ptable_entity_set_default_flags(&pspace_table, PTABLE_LV1);
        vm_ptable_entity_set_mmu_flags(&pspace_table, PTABLE_LV1, MMU_FLAG_PERM_READ | MMU_FLAG_PERM_WRITE | MMU_FLAG_UNCACHED);
        vm_ptable_entity_set_frame(&pspace_table, PTABLE_LV1, ptable_paddr_for);

        pdir->entities[VMM_OFFSET_IN_DIRECTORY(ptable_vaddr_for)] = pspace_table;
    }
    vmm_unmap_page_locked((uintptr_t)new_ptable);
    kmemzone_free(tmp_zone);
}

/**
 * @brief Frees pspace for pdir.
 */
void vm_pspace_free(ptable_t* pdir)
{
    ptable_entity_t* ptable_desc = &pdir->entities[VM_VADDR_OFFSET_AT_LEVEL(pspace_zone.start, PTABLE_LV_TOP)];
    if (!vm_ptable_entity_is_present(ptable_desc, PTABLE_LV_TOP)) {
        return;
    }
    vm_free_ptables_to_cover_page(vm_ptable_entity_get_frame(ptable_desc, PTABLE_LV_TOP));
    vm_ptable_entity_invalidate(ptable_desc, PTABLE_LV_TOP);
}

/**
 * @brief Updates pspace on ptable is created.
 *
 * @param vaddr The virtual address the new ptable is mapped to
 * @param ptable_paddr New ptable physical address
 * @param lv New ptable level
 * @return Status of the opertaion
 */
int vm_pspace_on_ptable_mapped(uintptr_t vaddr, uintptr_t ptable_paddr, ptable_lv_t lv)
{
    const uintptr_t pspace_table_vaddr = PAGE_START((uintptr_t)vm_pspace_get_vaddr_of_active_ptable(vaddr, lv));
    return vmm_map_page_locked(pspace_table_vaddr, ptable_paddr, MMU_FLAG_PERM_READ | MMU_FLAG_PERM_WRITE | MMU_FLAG_UNCACHED);
}

/**
 * @brief Returns pointer to an active ptable at lv level
 *
 * @param vaddr The virtual address to retrieve a ptable for
 * @param lv The level of the table that defines the target entity
 * @return Pointer to the ptable. Might return NULL if not found
 */
ptable_t* vm_get_table(uintptr_t vaddr, ptable_lv_t lv)
{
    if (!THIS_CPU->active_address_space) {
        return NULL;
    }

    ptable_t* active_pdir = THIS_CPU->active_address_space->pdir;
    ASSERT(active_pdir);

    if (lv == PTABLE_LV1) {
        return (ptable_t*)active_pdir;
    }

    ptable_entity_t* ptable_desc = vm_lookup(active_pdir, PTABLE_LV1, vaddr);
    if (!vm_ptable_entity_is_present(ptable_desc, PTABLE_LV1)) {
        return NULL;
    }

    return vm_pspace_get_vaddr_of_active_ptable(vaddr, lv);
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

    uintptr_t frame = vm_ptable_entity_get_frame(page, PTABLE_LV0);
    vm_ptable_entity_invalidate(page, PTABLE_LV0);

    memzone_t* zone = memzone_find_no_proc(zones, vaddr);
    if (zone) {
        if (zone->type & ZONE_TYPE_DEVICE) {
            return 0;
        }
    }

    vm_free_page_paddr(frame);
    return 0;
}

static int vm_pspace_free_ptable_locked(uintptr_t vaddr)
{
    // TODO: Free ptable and free page functions should be reimplemented with usage of level.
    vm_address_space_t* active_address_space = THIS_CPU->active_address_space;

    if (!active_address_space) {
        return -EACCES;
    }

    if (vmm_is_copy_on_write(vaddr)) {
        return -EFAULT;
    }

    ptable_entity_t* ptable_desc = vm_lookup(active_address_space->pdir, PTABLE_LV_TOP, vaddr);
    if (!vm_ptable_entity_is_present(ptable_desc, PTABLE_LV_TOP)) {
        return -EFAULT;
    }

    // Entering allocated state, since table is alloacted but not valid.
    // Allocated state will remove TABLE_DESC_PRESENT flag.
    uintptr_t frame = vm_ptable_entity_get_frame(ptable_desc, PTABLE_LV_TOP);
    vm_ptable_entity_allocated(ptable_desc, PTABLE_LV_TOP);
    vm_ptable_entity_set_frame(ptable_desc, PTABLE_LV_TOP, frame);

    ptable_t* ptable = vm_pspace_get_vaddr_of_active_ptable(vaddr, PTABLE_LV0);
    uintptr_t pages_vstart = TABLE_START(vaddr);
    for (uintptr_t i = 0, pages_voffset = 0; i < PTABLE_ENTITY_COUNT(PTABLE_LV0); i++, pages_voffset += VMM_PAGE_SIZE) {
        ptable_entity_t* page_desc = &ptable->entities[i];
        vm_pspace_free_page_locked(pages_vstart + pages_voffset, page_desc, &active_address_space->zones);
    }

    uintptr_t ptable_vaddr_start = PAGE_START((uintptr_t)ptable);
    size_t ptables_per_page = VMM_PAGE_SIZE / PTABLE_SIZE(PTABLE_LV0);
    size_t table_coverage = VMM_PAGE_SIZE * PTABLE_ENTITY_COUNT(PTABLE_LV0);
    uintptr_t ptable_serve_vaddr_start = (vaddr / (table_coverage * ptables_per_page)) * (table_coverage * ptables_per_page);

    // Check if it is possible to delete the whole page of tables.
    for (uintptr_t i = 0, pvaddr = ptable_serve_vaddr_start; i < ptables_per_page; i++, pvaddr += table_coverage) {
        ptable_entity_t* ptable_desc_c = vm_lookup(active_address_space->pdir, PTABLE_LV_TOP, pvaddr);
        if (vm_ptable_entity_is_present(ptable_desc, PTABLE_LV_TOP)) {
            return 0;
        }
    }

    // If we are here, we can delete the page of tables.
    ptable_entity_t* ptable_desc_first = vm_lookup(active_address_space->pdir, PTABLE_LV_TOP, ptable_serve_vaddr_start);
    vm_free_ptables_to_cover_page(vm_ptable_entity_get_frame(ptable_desc_first, PTABLE_LV_TOP));

    for (uintptr_t i = 0, pvaddr = ptable_serve_vaddr_start; i < ptables_per_page; i++, pvaddr += table_coverage) {
        ptable_entity_t* ptable_desc_c = vm_lookup(active_address_space->pdir, PTABLE_LV_TOP, pvaddr);
        vm_ptable_entity_invalidate(ptable_desc_c, PTABLE_LV_TOP);
    }

    // Cleaning Pspace
    vmm_unmap_page_locked(ptable_vaddr_start);
    return 0;
}

/**
 * @brief Clears address space, including all ptables.
 *
 * @param vm_aspace Address space to be cleared
 * @return Return status
 */
int vm_pspace_free_address_space_locked(vm_address_space_t* vm_aspace)
{
    vm_address_space_t* prev_aspace = vmm_get_active_address_space();
    vmm_switch_address_space_locked(vm_aspace);

    size_t table_coverage = VMM_PAGE_SIZE * PTABLE_ENTITY_COUNT(PTABLE_LV0);
    for (int i = 0; i < VMM_KERNEL_TABLES_START; i++) {
        vm_pspace_free_ptable_locked(table_coverage * i);
    }

    if (prev_aspace == vm_aspace) {
        vmm_switch_address_space_locked(vmm_get_kernel_address_space());
    } else {
        vmm_switch_address_space_locked(prev_aspace);
    }
    vm_pspace_free(vm_aspace->pdir);
    vm_free_ptable_lv_top(vm_aspace->pdir);
    return 0;
}