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
        table_desc_t* ptable_desc = _vmm_pdirectory_lookup(THIS_CPU->pdir, kernel_ptabels_vaddr);
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