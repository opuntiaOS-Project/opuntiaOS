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
#include <mem/vmm.h>
#include <platform/generic/cpu.h>
#include <platform/generic/system.h>
#include <platform/generic/vmm/mapping_table.h>
#include <platform/generic/vmm/pf_types.h>

/**
 * VM system for 32bit machines designed to support 2-level address
 * translation mechanism.
 * Pspace for 32bit arches is located right after the kernel and has
 * all ptables at the 2nd level mapped to this area. So, the size of
 * pspace is 4mb.
 *
 * TODO: Add detailed notes about the VM system design.
 */

static kmemzone_t pspace_zone;

ptable_t* vm_pspace_get_nth_active_ptable(size_t n)
{
    return (ptable_t*)(pspace_zone.start + n * PTABLE_SIZE);
}

ptable_t* vm_pspace_get_vaddr_of_active_ptable(uintptr_t vaddr)
{
    return (ptable_t*)vm_pspace_get_nth_active_ptable(VMM_OFFSET_IN_DIRECTORY(vaddr));
}

page_desc_t* vm_pspace_get_page_desc(uintptr_t vaddr)
{
    ptable_t* ptable = vm_pspace_get_vaddr_of_active_ptable(vaddr);
    return _vmm_ptable_lookup(ptable, vaddr);
}

/**
 * The function is used to init pspace.
 * Used only in the first stage of VM init.
 */
void vm_pspace_init()
{
    pspace_zone = kmemzone_new(4 * MB);

    if (VMM_OFFSET_IN_TABLE(pspace_zone.start) != 0) {
        kpanic("WRONG PSPACE START ADDR");
    }

    /* The code assumes that the length of tables which cover pspace
       is 4KB and that the tables are fit in a single page and are continuous. */
    extern uintptr_t kernel_ptables_start_paddr;
    const size_t ptables_per_page = VMM_PAGE_SIZE / PTABLE_SIZE;
    uintptr_t kernel_ptabels_vaddr = pspace_zone.start + VMM_KERNEL_TABLES_START * PTABLE_SIZE;
    uintptr_t kernel_ptabels_paddr = kernel_ptables_start_paddr;
    for (int i = VMM_KERNEL_TABLES_START; i < VMM_TOTAL_TABLES_PER_DIRECTORY; i += ptables_per_page) {
        table_desc_t* ptable_desc = _vmm_pdirectory_lookup(vmm_get_kernel_address_space()->pdir, kernel_ptabels_vaddr);
        if (!table_desc_is_present(*ptable_desc)) {
            kpanic("vm_pspace_init: pspace should present");
        }
        ptable_t* ptable_vaddr = (ptable_t*)(kernel_ptables_start_paddr + (VMM_OFFSET_IN_DIRECTORY(kernel_ptabels_vaddr) - VMM_KERNEL_TABLES_START) * PTABLE_SIZE);
        page_desc_t* page = _vmm_ptable_lookup(ptable_vaddr, kernel_ptabels_vaddr);
        page_desc_set_attrs(page, PAGE_DESC_PRESENT | PAGE_DESC_WRITABLE);
        page_desc_set_frame(page, kernel_ptabels_paddr);
        kernel_ptabels_vaddr += VMM_PAGE_SIZE;
        kernel_ptabels_paddr += VMM_PAGE_SIZE;
    }
}

/**
 * The function is used to generate a new pspace.
 * The function returns the table of itself.
 * Pspace table is one page (4KB) long, since the whole length is 4MB.
 */
void vm_pspace_gen(pdirectory_t* pdir)
{
    const uintptr_t ptables_per_page = VMM_PAGE_SIZE / PTABLE_SIZE;
    ptable_t* cur_ptable = vm_pspace_get_nth_active_ptable(VMM_OFFSET_IN_DIRECTORY(pspace_zone.start));
    uintptr_t ptable_paddr = vm_alloc_ptables_to_cover_page();
    kmemzone_t tmp_zone = kmemzone_new(VMM_PAGE_SIZE);
    ptable_t* new_ptable = (ptable_t*)tmp_zone.start;

    vmm_map_page_lockless((uintptr_t)new_ptable, ptable_paddr, MMU_FLAG_PERM_READ | MMU_FLAG_PERM_WRITE);

    /* The code assumes that the length of tables which cover pspace
       is 4KB and that the tables are fit in a single page and are continuous. */
    memcpy(new_ptable, cur_ptable, VMM_PAGE_SIZE);

    page_desc_t pspace_page;
    page_desc_init(&pspace_page);
    page_desc_set_attrs(&pspace_page, PAGE_DESC_PRESENT | PAGE_DESC_WRITABLE);
    page_desc_set_frame(&pspace_page, ptable_paddr);

    /* According to prev comment, we can remain overflow here, since we write to the right memory cell */
    new_ptable->entities[VMM_OFFSET_IN_DIRECTORY(pspace_zone.start) / ptables_per_page] = pspace_page;

    size_t table_coverage = VMM_PAGE_SIZE * VMM_TOTAL_PAGES_PER_TABLE;
    uintptr_t ptable_vaddr_for = pspace_zone.start;
    uintptr_t ptable_paddr_for = ptable_paddr;
    for (int i = 0; i < ptables_per_page; i++, ptable_vaddr_for += table_coverage, ptable_paddr_for += PTABLE_SIZE) {
        table_desc_t pspace_table;
        table_desc_init(&pspace_table);
        table_desc_set_attrs(&pspace_table, TABLE_DESC_PRESENT | TABLE_DESC_WRITABLE);
        table_desc_set_frame(&pspace_table, ptable_paddr_for);
        pdir->entities[VMM_OFFSET_IN_DIRECTORY(ptable_vaddr_for)] = pspace_table;
    }
    vmm_unmap_page_lockless((uintptr_t)new_ptable);
    kmemzone_free(tmp_zone);
}

void vm_pspace_free(pdirectory_t* pdir)
{
    table_desc_t* ptable_desc = &pdir->entities[VMM_OFFSET_IN_DIRECTORY(pspace_zone.start)];
    if (!table_desc_has_attrs(*ptable_desc, TABLE_DESC_PRESENT)) {
        return;
    }
    vm_free_ptables_to_cover_page(table_desc_get_frame(*ptable_desc));
    table_desc_del_frame(ptable_desc);
}

int vm_pspace_on_ptable_mapped(uintptr_t vaddr, uintptr_t ptable_paddr, ptable_lv_t level)
{
    const uintptr_t ptable_vaddr_start = PAGE_START((uintptr_t)vm_pspace_get_vaddr_of_active_ptable(vaddr));

    uint32_t ptable_settings = MMU_FLAG_PERM_READ | MMU_FLAG_PERM_WRITE | MMU_FLAG_PERM_EXEC;
    if (IS_USER_VADDR(vaddr)) {
        ptable_settings |= MMU_FLAG_NONPRIV;
    }

    return vmm_map_page_lockless(ptable_vaddr_start, ptable_paddr, ptable_settings);
}

ptable_t* vm_pspace_active_address_space_lookup(uintptr_t vaddr, ptable_lv_t lv)
{
    if (lv != PTABLE_LV0) {
        return NULL;
    }

    table_desc_t* ptable_desc = _vmm_pdirectory_lookup(THIS_CPU->active_address_space->pdir, vaddr);
    if (!table_desc_is_present(*ptable_desc)) {
        return NULL;
    }

    return vm_pspace_get_vaddr_of_active_ptable(vaddr);
}

// vm_pspace_free_page_lockless frees page.
static int vm_pspace_free_page_lockless(uintptr_t vaddr, page_desc_t* page, dynamic_array_t* zones)
{
    if (vmm_is_copy_on_write(vaddr)) {
        return -EBUSY;
    }

    if (!page_desc_has_attrs(*page, PAGE_DESC_PRESENT)) {
        return 0;
    }
    page_desc_del_attrs(page, PAGE_DESC_PRESENT);

    memzone_t* zone = memzone_find_no_proc(zones, vaddr);
    if (zone) {
        if (zone->type & ZONE_TYPE_DEVICE) {
            return 0;
        }
    }
    vm_free_page_paddr(page_desc_get_frame(*page));
    return 0;
}

// vm_pspace_free_ptable_lockless frees ptable and all data it contains of an
// active address space.
static int vm_pspace_free_ptable_lockless(uintptr_t vaddr)
{
    vm_address_space_t* active_address_space = THIS_CPU->active_address_space;

    if (!active_address_space) {
        return -EACCES;
    }

    if (vmm_is_copy_on_write(vaddr)) {
        return -EFAULT;
    }

    table_desc_t* ptable_desc = _vmm_pdirectory_lookup(active_address_space->pdir, vaddr);

    if (!table_desc_has_attrs(*ptable_desc, TABLE_DESC_PRESENT)) {
        return -EFAULT;
    }

    // Entering allocated state, since table is alloacted but not valid.
    // Allocated state will remove TABLE_DESC_PRESENT flag.
    uintptr_t frame = table_desc_get_frame(*ptable_desc);
    table_desc_set_allocated_state(ptable_desc);
    table_desc_set_frame(ptable_desc, frame);

    ptable_t* ptable = vm_pspace_get_vaddr_of_active_ptable(vaddr);
    uintptr_t pages_vstart = TABLE_START(vaddr);
    for (uintptr_t i = 0, pages_voffset = 0; i < VMM_TOTAL_PAGES_PER_TABLE; i++, pages_voffset += VMM_PAGE_SIZE) {
        page_desc_t* page = &ptable->entities[i];
        vm_pspace_free_page_lockless(pages_vstart + pages_voffset, page, &active_address_space->zones);
    }

    uintptr_t ptable_vaddr_start = PAGE_START((uintptr_t)ptable);
    size_t ptables_per_page = VMM_PAGE_SIZE / PTABLE_SIZE;
    size_t table_coverage = VMM_PAGE_SIZE * VMM_TOTAL_PAGES_PER_TABLE;
    uintptr_t ptable_serve_vaddr_start = (vaddr / (table_coverage * ptables_per_page)) * (table_coverage * ptables_per_page);

    // Chechking if we can delete thw whole page of tables.
    for (uintptr_t i = 0, pvaddr = ptable_serve_vaddr_start; i < ptables_per_page; i++, pvaddr += table_coverage) {
        table_desc_t* ptable_desc_c = _vmm_pdirectory_lookup(active_address_space->pdir, pvaddr);
        if (table_desc_has_attrs(*ptable_desc_c, TABLE_DESC_PRESENT)) {
            return 0;
        }
    }

    // If we are here, we can delete the page of tables.
    table_desc_t* ptable_desc_first = _vmm_pdirectory_lookup(active_address_space->pdir, ptable_serve_vaddr_start);
    vm_free_ptables_to_cover_page(table_desc_get_frame(*ptable_desc_first));

    for (uintptr_t i = 0, pvaddr = ptable_serve_vaddr_start; i < ptables_per_page; i++, pvaddr += table_coverage) {
        table_desc_t* ptable_desc_c = _vmm_pdirectory_lookup(active_address_space->pdir, pvaddr);
        table_desc_clear(ptable_desc_c);
    }

    // Cleaning Pspace
    vmm_unmap_page_lockless(ptable_vaddr_start);
    return 0;
}

// vm_pspace_free_address_space_lockless removes all ptables and data
// associated with current address space.
int vm_pspace_free_address_space_lockless(vm_address_space_t* vm_aspace)
{
    vm_address_space_t* prev_aspace = vmm_get_active_address_space();
    vmm_switch_address_space_lockless(vm_aspace);

    size_t table_coverage = VMM_PAGE_SIZE * VMM_TOTAL_PAGES_PER_TABLE;
    for (int i = 0; i < VMM_KERNEL_TABLES_START; i++) {
        vm_pspace_free_ptable_lockless(table_coverage * i);
    }

    if (prev_aspace == vm_aspace) {
        vmm_switch_address_space_lockless(vmm_get_kernel_address_space());
    } else {
        vmm_switch_address_space_lockless(prev_aspace);
    }
    vm_pspace_free(vm_aspace->pdir);
    vm_free_ptable_lv_top((pte_t*)vm_aspace->pdir);
    return 0;
}