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
#include <mem/kmalloc.h>
#include <mem/kmemzone.h>
#include <mem/swapfile.h>
#include <mem/vm_alloc.h>
#include <mem/vm_pspace.h>
#include <mem/vmm.h>
#include <platform/generic/cpu.h>
#include <platform/generic/system.h>
#include <platform/generic/vmm/mapping_table.h>
#include <tasking/tasking.h>

// #define VMM_DEBUG
// #define VMM_DEBUG_SWAP

static vm_address_space_t _vmm_kernel_address_space;
static vm_address_space_t* _vmm_kernel_address_space_ptr = &_vmm_kernel_address_space;
static uintptr_t _vmm_kernel_pdir_paddr;
static ptable_t* _vmm_kernel_pdir;
static spinlock_t _vmm_global_lock;
static kmemzone_t pspace_zone;
uintptr_t kernel_ptables_start_paddr = 0x0;

/**
 * PRIVATE FUNCTIONS
 */

static int _vmm_allocate_ptable(uintptr_t vaddr, ptable_lv_t lv);

static void* _vmm_kernel_convert_vaddr2paddr(uintptr_t vaddr);
static void* _vmm_convert_vaddr2paddr(uintptr_t vaddr);

static bool _vmm_init_switch_to_kernel_pdir();
static void _vmm_map_init_kernel_pages(uintptr_t paddr, uintptr_t vaddr);
static bool _vmm_create_kernel_ptables();
static bool _vmm_map_kernel();

int vmm_resolve_copy_on_write(uintptr_t vaddr);
static int _vmm_copy_page_to_resolve_cow(uintptr_t vaddr, ptable_entity_t* old_page_desc);

static bool _vmm_is_page_swapped_entity(ptable_entity_t* page_desc);
static bool _vmm_is_page_swapped(uintptr_t vaddr);
static int _vmm_restore_swapped_page_locked(uintptr_t vaddr);

static int _vmm_self_test();
static void _vmm_dump_page(uintptr_t vaddr);

/**
 * LOCKLESS FUNCTIONS
 */

static int _vmm_allocate_ptable_locked(uintptr_t vaddr, ptable_lv_t lv);
static int _vmm_force_allocate_ptable_locked(uintptr_t vaddr, ptable_lv_t lv);
static int _vmm_free_address_space_locked(vm_address_space_t* vm_aspace);

static vm_address_space_t* vmm_new_address_space_locked();
static vm_address_space_t* vmm_new_forked_address_space_locked();
static ALWAYS_INLINE void vmm_ensure_writing_to_active_address_space_locked(uintptr_t dest_vaddr, size_t length);

/**
 * VM INITIALIZATION FUNCTIONS
 */

/**
 * @note Called only during the first stage of VM init.
 */
static void vm_alloc_kernel_pdir()
{
    _vmm_kernel_pdir_paddr = (uintptr_t)pmm_alloc_aligned(PTABLE_SIZE(PTABLE_LV_TOP), PTABLE_SIZE(PTABLE_LV_TOP));
    _vmm_kernel_pdir = (ptable_t*)(_vmm_kernel_pdir_paddr + pmm_get_state()->boot_args->vaddr - pmm_get_state()->boot_args->paddr);
    _vmm_kernel_address_space.count = 1;
    _vmm_kernel_address_space.pdir = _vmm_kernel_pdir;
    spinlock_init(&_vmm_kernel_address_space.lock);
    memset((void*)_vmm_kernel_address_space.pdir, 0, PTABLE_SIZE(PTABLE_LV_TOP));

    // Set as an active one to set up kernel address space.
    THIS_CPU->active_address_space = _vmm_kernel_address_space_ptr;
}

/**
 * @brief Traslates virtual address into physical.
 *
 * @note Called only during the first stage of VM init.
 */
static void* _vmm_kernel_convert_vaddr2paddr(uintptr_t vaddr)
{
    ptable_t* ptable_paddr = (ptable_t*)(kernel_ptables_start_paddr + (VMM_OFFSET_IN_DIRECTORY(vaddr) - VMM_KERNEL_TABLES_START) * PTABLE_SIZE(PTABLE_LV0));
    ptable_entity_t* page_desc = vm_lookup(ptable_paddr, PTABLE_LV0, vaddr);
    return (void*)((vm_ptable_entity_get_frame(page_desc, PTABLE_LV0)) | (vaddr & 0xfff));
}

/**
 * @brief Traslates virtual address into physical.
 */
static void* _vmm_convert_vaddr2paddr(uintptr_t vaddr)
{
    ptable_entity_t* page_desc = vm_get_entity(vaddr, PTABLE_LV0);
    return (void*)((vm_ptable_entity_get_frame(page_desc, PTABLE_LV0)) | (vaddr & 0xfff));
}

/**
 * @note Called only during the first stage of VM init.
 */
static bool _vmm_init_switch_to_kernel_pdir()
{
    THIS_CPU->active_address_space = _vmm_kernel_address_space_ptr;
    system_disable_interrupts();
    system_set_pdir((uintptr_t)_vmm_kernel_convert_vaddr2paddr((uintptr_t)_vmm_kernel_address_space.pdir));
    system_enable_interrupts();
    return true;
}

/**
 * @brief Maps kernel pages.
 *
 * @note Called only during the first stage of VM init.
 */
static void _vmm_map_kernel_page(uintptr_t paddr, uintptr_t vaddr)
{
    ptable_t* ptable_paddr = (ptable_t*)(kernel_ptables_start_paddr + (VMM_OFFSET_IN_DIRECTORY(vaddr) - VMM_KERNEL_TABLES_START) * PTABLE_SIZE(PTABLE_LV0));
    ptable_entity_t* page_desc = vm_lookup(ptable_paddr, PTABLE_LV0, vaddr);
    vm_ptable_entity_set_default_flags(page_desc, PTABLE_LV0);
    vm_ptable_entity_set_mmu_flags(page_desc, PTABLE_LV0, MMU_FLAG_PERM_READ | MMU_FLAG_PERM_WRITE | MMU_FLAG_PERM_EXEC);
    vm_ptable_entity_set_frame(page_desc, PTABLE_LV0, paddr);
}

/**
 * @brief Creates all kernel tables and map necessary data into _vmm_kernel_pdir.
 *
 * @note Called only during the first stage of VM init.
 */
static bool _vmm_create_kernel_ptables()
{
    const ptable_lv_t ptable_entity_lv = PTABLE_LV_TOP;
    const size_t ptables_per_page = VMM_PAGE_SIZE / PTABLE_SIZE(lower_level(ptable_entity_lv));
    const size_t table_coverage = VMM_PAGE_SIZE * PTABLE_ENTITY_COUNT(lower_level(ptable_entity_lv));
    uintptr_t kernel_ptables_vaddr = VMM_KERNEL_TABLES_START * table_coverage;

    // Tables allocated here should be continuous to correctly generate
    // pspace, see vm_pspace_init().
    const size_t need_pages_for_kernel_ptables = PTABLE_ENTITY_COUNT(ptable_entity_lv) / ptables_per_page;
    uintptr_t ptables_paddr = (uintptr_t)pmm_alloc_aligned(need_pages_for_kernel_ptables * VMM_PAGE_SIZE, VMM_PAGE_SIZE);
    if (!ptables_paddr) {
        kpanic("_vmm_create_kernel_ptables: No free space in pmm to alloc kernel ptables");
    }
    kernel_ptables_start_paddr = ptables_paddr;

    uintptr_t ptable_paddr = ptables_paddr;
    for (int i = VMM_KERNEL_TABLES_START; i < PTABLE_ENTITY_COUNT(ptable_entity_lv); i += ptables_per_page) {
        ptable_entity_t* ptable_entity = vm_get_entity(kernel_ptables_vaddr, ptable_entity_lv);
        vm_ptable_entity_set_default_flags(ptable_entity, ptable_entity_lv);
        vm_ptable_entity_set_mmu_flags(ptable_entity, ptable_entity_lv, MMU_FLAG_PERM_READ | MMU_FLAG_PERM_WRITE);
        vm_ptable_entity_set_frame(ptable_entity, ptable_entity_lv, ptable_paddr);

        ptable_paddr += PTABLE_SIZE(lower_level(ptable_entity_lv));
        kernel_ptables_vaddr += table_coverage;

        // Kernel tables and pspace are not marked as user, but following
        // tables are marked as user, to allow user access some pages, like
        // signal trampolines.
        // TODO: Implement a seperate table for user pages inside kernel.
        // This will allow to mark only sepcial tables as user, while all others
        // are protected.
        if (i > VMM_OFFSET_IN_DIRECTORY(pspace_zone.start)) {
            vm_ptable_entity_set_mmu_flags(ptable_entity, ptable_entity_lv, MMU_FLAG_NONPRIV);
        }
    }

    const pmm_state_t* pmm_state = pmm_get_state();
    uintptr_t paddr = pmm_state->boot_args->paddr;
    uintptr_t vaddr = pmm_state->kernel_va_base;
    uintptr_t end_vaddr = pmm_state->kernel_va_base + pmm_state->kernel_data_size;
    while (vaddr < end_vaddr) {
        _vmm_map_kernel_page(paddr, vaddr);
        vaddr += VMM_PAGE_SIZE;
        paddr += VMM_PAGE_SIZE;
    }

    for (uintptr_t iaddr = 0; iaddr < PTABLE_SIZE(PTABLE_LV_TOP); iaddr += VMM_PAGE_SIZE) {
        _vmm_map_kernel_page(_vmm_kernel_pdir_paddr + iaddr, (uintptr_t)_vmm_kernel_pdir + iaddr);
    }

    return true;
}

/**
 * @brief Maps secondary data for kernel's pdir.
 *
 * @note Called only during the first stage of VM init.
 */
static bool _vmm_map_kernel()
{
    int te = 0;
    do {
        vmm_map_pages(extern_mapping_table[te].paddr, extern_mapping_table[te].vaddr, extern_mapping_table[te].pages, extern_mapping_table[te].flags);
    } while (!extern_mapping_table[te++].last);
    return true;
}

/**
 * VM SETUP
 */

int vmm_init_setup_finished = 0;

int vmm_setup(boot_args_t* args)
{
    spinlock_init(&_vmm_global_lock);
    kmemzone_init();
    vm_alloc_kernel_pdir();
    _vmm_create_kernel_ptables();
    vm_pspace_init(args);
    _vmm_init_switch_to_kernel_pdir();
    _vmm_map_kernel();
    kmemzone_init_stage2();
    kmalloc_init();

    // After kmalloc is set up, we can allocate dynarr for zones.
    dynarr_init_of_size(memzone_t, &_vmm_kernel_address_space.zones, 4);
    vmm_init_setup_finished = 1;
    return 0;
}

int vmm_setup_secondary_cpu()
{
    _vmm_init_switch_to_kernel_pdir();
    return 0;
}

/**
 * VM TOOLS
 */

static inline void _vmm_sleep_locked()
{
    // Double-check that idle thread does not allocate and not preempt.
    // Look at the comment about idle thread in sched.c.
    ASSERT(RUNNING_THREAD != THIS_CPU->idle_thread);
    extern void resched();

    vm_address_space_t* active_address_space = vmm_get_active_address_space();
    spinlock_release(&active_address_space->lock);
    resched();
    spinlock_acquire(&active_address_space->lock);
}

static inline void _vmm_table_desc_init_from_allocated_state(ptable_entity_t* ptable_desc, ptable_lv_t lv)
{
    uintptr_t frame = vm_ptable_entity_get_frame(ptable_desc, lv);
    vm_ptable_entity_set_default_flags(ptable_desc, lv);
    vm_ptable_entity_set_mmu_flags(ptable_desc, lv, MMU_FLAG_PERM_READ | MMU_FLAG_PERM_WRITE | MMU_FLAG_PERM_EXEC | MMU_FLAG_NONPRIV);
    vm_ptable_entity_set_frame(ptable_desc, lv, frame);
}

/**
 * @brief Sets up a new ptable.
 *
 * @param vaddr The virtual address the new ptable serves.
 * @param ptable_lv New ptable level.
 * @return Status of the operation.
 *
 * It may allocate several tables to cover the full page. For example,
 * it allocates 4 tables for arm (ptable -- 1KB, while a page is 4KB).
 */
static int _vmm_allocate_ptable_locked(uintptr_t vaddr, ptable_lv_t ptable_lv)
{
    if (!THIS_CPU->active_address_space) {
        return -EACCES;
    }

    ptable_lv_t ptable_desc_lv = upper_level(ptable_lv);
    ptable_entity_t* ptable_desc = vm_get_entity(vaddr, ptable_desc_lv);
    if (vm_ptable_entity_is_only_allocated(ptable_desc, ptable_desc_lv)) {
        goto skip_allocation;
    }

    uintptr_t ptables_paddr = 0;
    do {
        ptables_paddr = vm_alloc_ptables_to_cover_page();
        if (!ptables_paddr) {
            _vmm_sleep_locked();
        }
    } while (!ptables_paddr);

    const size_t ptables_per_page = VMM_PAGE_SIZE / PTABLE_SIZE(ptable_lv);
    const size_t table_coverage = VMM_PAGE_SIZE * PTABLE_ENTITY_COUNT(ptable_lv);
    uintptr_t ptable_serve_vaddr_start = (vaddr / (table_coverage * ptables_per_page)) * (table_coverage * ptables_per_page);

    for (uintptr_t i = 0, pvaddr = ptable_serve_vaddr_start, ptable_paddr = ptables_paddr; i < ptables_per_page; i++) {
        ptable_entity_t* tmp_ptable_desc = vm_get_entity(pvaddr, ptable_desc_lv);
        vm_ptable_entity_allocated(tmp_ptable_desc, ptable_desc_lv);
        vm_ptable_entity_set_frame(tmp_ptable_desc, ptable_desc_lv, ptable_paddr);

        pvaddr += table_coverage;
        ptable_paddr += PTABLE_SIZE(ptable_lv);
    }

    int err = vm_pspace_on_ptable_mapped(vaddr, ptables_paddr, ptable_lv);
    if (err) {
        return err;
    }

skip_allocation:
    _vmm_table_desc_init_from_allocated_state(ptable_desc, ptable_desc_lv);
    memset(vm_get_table(vaddr, ptable_lv), 0, PTABLE_SIZE(ptable_lv));
    return 0;
}

/**
 * @brief Sets up a new ptable.
 *
 * @param vaddr The virtual address the new ptable serves.
 * @param ptable_lv New ptable level.
 * @return Status of operation.
 */
static int _vmm_allocate_ptable(uintptr_t vaddr, ptable_lv_t lv)
{
    vm_address_space_t* active_address_space = vmm_get_active_address_space();
    spinlock_acquire(&active_address_space->lock);
    int res = _vmm_allocate_ptable_locked(vaddr, lv);
    spinlock_release(&active_address_space->lock);
    return res;
}

/**
 * @brief Sets up a new ptable, ignoring pre-allocated state.
 *
 * @param vaddr The virtual address the new ptable serves.
 * @param ptable_lv New ptable level.
 * @return Status of operation.
 */
static int _vmm_force_allocate_ptable_locked(uintptr_t vaddr, ptable_lv_t lv)
{
    // Clearing Allocated state flags, so vmm_allocate_ptable will allocate new tables.
    ptable_entity_t* ptable_desc = vm_get_entity(vaddr, upper_level(lv));
    vm_ptable_entity_invalidate(ptable_desc, upper_level(lv));
    return _vmm_allocate_ptable_locked(vaddr, lv);
}

/**
 * @brief Sets up a new ptable, ignoring pre-allocated state.
 *
 * @param vaddr The virtual address the new ptable serves.
 * @param ptable_lv New ptable level.
 * @return Status of operation.
 */
static int vmm_force_allocate_ptable(uintptr_t vaddr, ptable_lv_t lv)
{
    vm_address_space_t* active_address_space = vmm_get_active_address_space();
    spinlock_acquire(&active_address_space->lock);
    int res = _vmm_force_allocate_ptable_locked(vaddr, lv);
    spinlock_release(&active_address_space->lock);
    return res;
}

/**
 * @brief Checks if page is present.
 *
 * @param vaddr The virtual address to examine.
 * @return True if page is present.
 */
bool vmm_is_page_present_impl(uintptr_t vaddr)
{
    ptable_entity_t* page_desc = vm_get_entity(vaddr, PTABLE_LV0);
    if (!page_desc) {
        return false;
    }
    return vm_ptable_entity_is_present(page_desc, PTABLE_LV0);
}

/**
 * @brief Maps a page specified with addresses.
 *
 * @param vaddr The virtual address to map.
 * @param paddr The physical address to map to.
 * @param mmu_flags Permission flags to map with.
 * @return Status of the operation.
 */
int vmm_map_page_locked_impl(uintptr_t vaddr, uintptr_t paddr, mmu_flags_t mmu_flags)
{
    if (!THIS_CPU->active_address_space) {
        return -EACCES;
    }

    vaddr = ROUND_FLOOR(vaddr, VMM_PAGE_SIZE);
    if (vmm_is_copy_on_write(vaddr)) {
        return -EBUSY;
    }

    // TODO: Currently we allocate only LV0 table, since we support only 2-level translation for now.
    ptable_lv_t ptable_level = PTABLE_LV0;
    ptable_entity_t* ptable_desc = vm_get_entity(vaddr, upper_level(ptable_level));
    if (!vm_ptable_entity_is_present(ptable_desc, upper_level(ptable_level))) {
        _vmm_allocate_ptable_locked(vaddr, ptable_level);
        ptable_desc = vm_get_entity(vaddr, upper_level(ptable_level));
    }

    ptable_entity_t* page_desc = vm_get_entity(vaddr, PTABLE_LV0);
    vm_ptable_entity_set_default_flags(page_desc, PTABLE_LV0);
    vm_ptable_entity_set_mmu_flags(page_desc, PTABLE_LV0, mmu_flags | MMU_FLAG_PERM_READ);
    vm_ptable_entity_set_frame(page_desc, PTABLE_LV0, paddr);

#ifdef VMM_DEBUG
    log("Page mapped %x in pdir: %x", vaddr, vmm_get_active_pdir());
#endif

    system_flush_local_tlb_entry(vaddr);
    return 0;
}

/**
 * @brief Unmaps a page specified with addresses.
 *
 * @param vaddr The virtual address to unmap.
 * @return Status of the operation.
 */
int vmm_unmap_page_locked_impl(uintptr_t vaddr)
{
    if (!THIS_CPU->active_address_space) {
        return -EACCES;
    }

    if (vmm_is_copy_on_write(vaddr)) {
        return -EBUSY;
    }

    ptable_entity_t* page_desc = vm_get_entity(vaddr, PTABLE_LV0);
    if (!vm_ptable_entity_is_present(page_desc, PTABLE_LV0)) {
        return -EACCES;
    }

    vm_ptable_entity_invalidate(page_desc, PTABLE_LV0);
    system_flush_local_tlb_entry(vaddr);
    return 0;
}

/**
 * CoW FUNCTIONS
 */

extern int vmm_alloc_page_locked(uintptr_t vaddr, mmu_flags_t mmu_flags);

/**
 * @brief Marks both page tables as COW.
 */
static void _vmm_tables_set_cow(size_t table_index, ptable_entity_t* cur, ptable_entity_t* new, ptable_lv_t lv)
{
    // TODO: Support 2-level translation for now only.
    // Reimplement getting a target pdir and moving levels down.
    vm_ptable_entity_set_mmu_flags(cur, lv, MMU_FLAG_COW);
    vm_ptable_entity_set_mmu_flags(new, lv, MMU_FLAG_COW);

    // Marking all pages as not-writable to handle COW. Later will restore using zones data.
    ptable_t* ptable = vm_pspace_get_nth_active_ptable(table_index, lower_level(lv));
    for (int i = 0; i < PTABLE_ENTITY_COUNT(lower_level(lv)); i++) {
        ptable_entity_t* page = &ptable->entities[i];
        if (vm_ptable_entity_is_present(page, lower_level(lv))) {
            vm_ptable_entity_rm_mmu_flags(page, lower_level(lv), MMU_FLAG_PERM_WRITE);
        }
    }
}

/**
 * @brief Copies data from one page to another to resolve CoW
 * for an active address space.
 *
 * @note Only to resolve CoW.
 */
static int _vmm_copy_page_to_resolve_cow(uintptr_t vaddr, ptable_entity_t* old_page_desc)
{
    vm_address_space_t* active_address_space = vmm_get_active_address_space();
    if (!active_address_space) {
        return -EINVAL;
    }

    memzone_t* zone = memzone_find(active_address_space, vaddr);
    if (!zone) {
        kpanic("Cow: No page in zone");
        return -EFAULT;
    }

    if (TEST_FLAG(zone->type, ZONE_TYPE_DEVICE) || TEST_FLAG(zone->type, ZONE_TYPE_MAPPED_FILE_SHAREDLY)) {
        uintptr_t old_page_paddr = vm_ptable_entity_get_frame(old_page_desc, PTABLE_LV0);
        return vmm_map_page_locked(vaddr, old_page_paddr, zone->mmu_flags);
    }

    vmm_alloc_page_locked(vaddr, zone->mmu_flags);

    /* Mapping the old page to do a copy */
    kmemzone_t tmp_zone = kmemzone_new(VMM_PAGE_SIZE);
    uintptr_t old_page_vaddr = (uintptr_t)tmp_zone.start;
    uintptr_t old_page_paddr = vm_ptable_entity_get_frame(old_page_desc, PTABLE_LV0);
    int err = vmm_map_page_locked(old_page_vaddr, old_page_paddr, MMU_FLAG_PERM_READ);
    if (err) {
        return err;
    }

    // We don't need to check if there is a problem with cow, since this is a special copy function
    // to resolve cow. So, we know that pages were created recently. Checking cow here would cause an
    // ub, since table has not been setup correctly yet.
    memcpy((void*)vaddr, (void*)old_page_vaddr, VMM_PAGE_SIZE);

    vmm_unmap_page_locked(old_page_vaddr);
    kmemzone_free(tmp_zone);
    return 0;
}

static bool vmm_is_copy_on_write_impl(uintptr_t vaddr, ptable_lv_t lv)
{
    ptable_entity_t* pdesc = vm_get_entity(vaddr, lv);
    mmu_flags_t mmu_flags = vm_arch_to_mmu_flags(pdesc, lv);
    if (TEST_FLAG(mmu_flags, MMU_FLAG_COW)) {
        return true;
    }

    if (lv != PTABLE_LV0 && vm_ptable_entity_is_present(pdesc, lv)) {
        return vmm_is_copy_on_write_impl(vaddr, lower_level(lv));
    }

    return false;
}

/**
 * @brief Checks if vaddr of an active address space marked as CoW.
 *
 * @param vaddr The virtual address to examine.
 * @return Boolean, showing if marked as CoW.
 */
bool vmm_is_copy_on_write(uintptr_t vaddr)
{
    return vmm_is_copy_on_write_impl(vaddr, PTABLE_LV_TOP);
}

/**
 * @brief Resolves CoW for an active address space.
 */
int vmm_resolve_copy_on_write(uintptr_t vaddr)
{
    ptable_lv_t lv = PTABLE_LV_TOP;
    ptable_lv_t entity_lv = lv;
    const size_t ptables_per_page = VMM_PAGE_SIZE / PTABLE_SIZE(lower_level(lv));
    ptable_entity_t orig_table_desc[ptables_per_page];

    size_t table_coverage = VMM_PAGE_SIZE * PTABLE_ENTITY_COUNT(lower_level(lv));
    uintptr_t ptable_serve_vaddr_start = (vaddr / (table_coverage * ptables_per_page)) * (table_coverage * ptables_per_page);

    // Copying old ptables which cover the full page. See a comment above vmm_allocate_ptable.
    kmemzone_t src_ptable_zone;
    int err = 0;
    do {
        err = vm_alloc_mapped_zone(VMM_PAGE_SIZE, VMM_PAGE_SIZE, &src_ptable_zone);
        if (err) {
            _vmm_sleep_locked();
        }
    } while (err);

    ptable_t* src_ptable = (ptable_t*)src_ptable_zone.ptr;
    ptable_t* root_ptable = (ptable_t*)PAGE_START((uintptr_t)vm_get_table(vaddr, lower_level(lv)));
    memcpy(src_ptable, root_ptable, VMM_PAGE_SIZE);

    // Saving descriptors of original ptables
    ptable_entity_t* tmp_table_desc = vm_get_entity(ptable_serve_vaddr_start, entity_lv);
    for (int it = 0; it < ptables_per_page; it++) {
        orig_table_desc[it] = tmp_table_desc[it];
    }

    // Setting up new ptables.
    err = _vmm_force_allocate_ptable_locked(vaddr, lower_level(lv));
    if (err) {
        return err;
    }

    // Copying all pages from this tables to newly allocated.
    uintptr_t table_start = TABLE_START(ptable_serve_vaddr_start);
    ptable_entity_t* start_ptable_desc = vm_get_entity(ptable_serve_vaddr_start, entity_lv);
    for (int ptable_idx = 0; ptable_idx < ptables_per_page; ptable_idx++) {
        if (!vm_ptable_entity_is_present(&orig_table_desc[ptable_idx], entity_lv)) {
            continue;
        }

        _vmm_table_desc_init_from_allocated_state(&start_ptable_desc[ptable_idx], entity_lv);
        for (int page_idx = 0; page_idx < PTABLE_ENTITY_COUNT(lower_level(lv)); page_idx++) {
            size_t offset_in_table_set = ptable_idx * PTABLE_ENTITY_COUNT(lower_level(lv)) + page_idx;
            uintptr_t page_vaddr = table_start + (offset_in_table_set * VMM_PAGE_SIZE);

            ptable_entity_t* page_desc = &src_ptable->entities[offset_in_table_set];
            if (_vmm_is_page_swapped_entity(page_desc)) {
                ptable_entity_t* current_page_desc = vm_get_entity(page_vaddr, PTABLE_LV0);
                ASSERT(current_page_desc); // Double-check that the page descriptor is already allocated.

                uintptr_t id = ((vm_ptable_entity_get_frame(page_desc, PTABLE_LV0)) >> PAGE_DESC_FRAME_OFFSET);
#ifdef VMM_DEBUG_SWAP
                log("   %d :: new ref for swapped %x == id %d", RUNNING_THREAD->tid, page_vaddr, id);
#endif
                swapfile_new_ref(id);
                *current_page_desc = *page_desc;
                continue;
            }

            if (!vm_ptable_entity_is_present(page_desc, lower_level(lv))) {
                continue;
            }

            err = _vmm_copy_page_to_resolve_cow(page_vaddr, page_desc);
            if (err) {
                return err;
            }
        }
    }

    return vm_free_mapped_zone(src_ptable_zone);
}

/**
 * SWAP FUNCTIONS
 */

extern int vm_alloc_user_page_no_fill_locked(memzone_t* zone, uintptr_t vaddr);
extern memzone_t* vmm_memzone_for_active_address_space(uintptr_t vaddr);

static bool _vmm_is_page_swapped_entity(ptable_entity_t* page_desc)
{
    if (!page_desc) {
        return false;
    }

    if (vm_ptable_entity_is_present(page_desc, PTABLE_LV0)) {
        return false;
    }

    if (!vm_ptable_entity_get_frame(page_desc, PTABLE_LV0)) {
        return false;
    }

    return true;
}

bool vmm_is_page_swapped_impl(uintptr_t vaddr)
{
    ptable_entity_t* page_desc = vm_get_entity(vaddr, PTABLE_LV0);
    return _vmm_is_page_swapped_entity(page_desc);
}

int vmm_restore_swapped_page_locked_impl(uintptr_t vaddr)
{
    memzone_t* zone = vmm_memzone_for_active_address_space(vaddr);
    if (!zone) {
        return -1;
    }

    int swap_type = SWAP_TO_DEV;

    ptable_entity_t* page = vm_get_entity(vaddr, PTABLE_LV0);
    uintptr_t id = ((vm_ptable_entity_get_frame(page, PTABLE_LV0)) >> PAGE_DESC_FRAME_OFFSET);

    int err = vm_alloc_user_page_no_fill_locked(zone, vaddr);
    if (err) {
        return err;
    }

    err = swapfile_load(vaddr, id);
    if (err) {
        return err;
    }

    system_flush_all_cpus_tlb_entry(vaddr);

#ifdef VMM_DEBUG_SWAP
    uint32_t checksum = 0;
    uint32_t* old_page_vaddr = (uint32_t*)PAGE_START(vaddr);
    for (int i = 0; i < VMM_PAGE_SIZE / 4; i++) {
        checksum ^= old_page_vaddr[i];
    }
    log("Swap: %d restore %x == id %d (%x *%x) chksm %x", RUNNING_THREAD->tid, vaddr, id, page, *page, checksum);
#endif
    return 0;
}

int vmm_swap_page_impl(ptable_entity_t* page_desc, memzone_t* zone, uintptr_t vaddr)
{
    vm_address_space_t* active_address_space = vmm_get_active_address_space();
    spinlock_acquire(&active_address_space->lock);

    if (!zone) {
        spinlock_release(&active_address_space->lock);
        return -EINVAL;
    }

    int swap_mode = SWAP_TO_DEV;
    if (zone->ops && zone->ops->swap_page_mode) {
        swap_mode = zone->ops->swap_page_mode(zone, vaddr);
    }

    if (swap_mode == SWAP_NOT_ALLOWED) {
        spinlock_release(&active_address_space->lock);
        return -EPERM;
    }

    kmemzone_t tmp_zone = kmemzone_new(VMM_PAGE_SIZE);
    uintptr_t old_page_vaddr = (uintptr_t)tmp_zone.start;
    uintptr_t old_page_paddr = vm_ptable_entity_get_frame(page_desc, PTABLE_LV0);
    int err = vmm_map_page_locked(old_page_vaddr, old_page_paddr, MMU_FLAG_PERM_READ);
    if (err) {
        spinlock_release(&active_address_space->lock);
        return err;
    }

    int new_frame = 0;
    if (swap_mode == SWAP_TO_DEV) {
        new_frame = swapfile_store(old_page_vaddr);
        if (new_frame < 0) {
            vmm_unmap_page_locked(old_page_vaddr);
            kmemzone_free(tmp_zone);
            spinlock_release(&active_address_space->lock);
            return -1;
        }
    }

#ifdef VMM_DEBUG_SWAP
    uint32_t checksum = 0;
    uint32_t* old_page_vaddr = (uint32_t*)PAGE_START(old_page_vaddr);
    for (int i = 0; i < VMM_PAGE_SIZE / 4; i++) {
        checksum ^= old_page_vaddr[i];
    }
    log("Swap: %d put to swap %x == id %d (%x *%x) chksm %x", RUNNING_THREAD->tid, vaddr, new_frame, page_desc, *page_desc, checksum);
#endif

    vm_free_page_paddr(vm_ptable_entity_get_frame(page_desc, PTABLE_LV0));
    vm_ptable_entity_invalidate(page_desc, PTABLE_LV0);
    vm_ptable_entity_set_frame(page_desc, PTABLE_LV0, new_frame << PAGE_DESC_FRAME_OFFSET);
    system_flush_all_cpus_tlb_entry(vaddr);

    vmm_unmap_page_locked(old_page_vaddr);
    kmemzone_free(tmp_zone);
    spinlock_release(&active_address_space->lock);
    return 0;
}

/**
 * ADDRESS SPACE FUNCTIONS
 */

vm_address_space_t* vmm_alloc_new_address_space_locked()
{
    vm_address_space_t* new_address_space = vm_address_space_alloc();
    ptable_t* new_pdir;
    do {
        new_pdir = vm_alloc_ptable_lv_top();
        if (!new_pdir) {
            _vmm_sleep_locked();
        }
    } while (!new_pdir);

    new_address_space->pdir = new_pdir;
    return new_address_space;
}

int vmm_fill_up_new_address_space(vm_address_space_t* new_aspace)
{
    for (int i = 0; i < VMM_KERNEL_TABLES_START; i++) {
        vm_ptable_entity_invalidate(&new_aspace->pdir->entities[i], PTABLE_LV_TOP);
    }

    for (int i = VMM_KERNEL_TABLES_START; i < PTABLE_ENTITY_COUNT(PTABLE_LV_TOP); i++) {
        if (!IS_INDIVIDUAL_PER_DIR(i)) {
            new_aspace->pdir->entities[i] = _vmm_kernel_pdir->entities[i];
        }
    }

    vm_pspace_gen(new_aspace->pdir);
    return 0;
}

int vmm_fill_up_forked_address_space(vm_address_space_t* new_aspace)
{
    vm_address_space_t* active_address_space = THIS_CPU->active_address_space;
    spinlock_acquire(&active_address_space->lock);

    // Copying the top-most entries to the new table.
    for (int i = 0; i < PTABLE_ENTITY_COUNT(PTABLE_LV_TOP); i++) {
        new_aspace->pdir->entities[i] = active_address_space->pdir->entities[i];
    }
    vm_pspace_gen(new_aspace->pdir);

    for (int i = 0; i < VMM_KERNEL_TABLES_START; i++) {
        ptable_entity_t* act_ptable_desc = &active_address_space->pdir->entities[i];
        if (vm_ptable_entity_is_present(act_ptable_desc, PTABLE_LV_TOP)) {
            ptable_entity_t* new_ptable_desc = &new_aspace->pdir->entities[i];
            _vmm_tables_set_cow(i, act_ptable_desc, new_ptable_desc, PTABLE_LV_TOP);
        }
    }

    system_flush_whole_tlb();
    spinlock_release(&active_address_space->lock);
    return 0;
}

/**
 * @brief Frees address space. If target vm_aspace is an active address space,
 *        after deletion kernel will be moved to kernel address space.
 *
 * @param vm_aspace The address space for deletion.
 * @return Status of the operation.
 */
int vmm_free_address_space_locked_impl(vm_address_space_t* vm_aspace)
{
    return vm_pspace_free_address_space_locked(vm_aspace);
}

vm_address_space_t* vmm_get_active_address_space()
{
    // At first glance we don't need lock here
    return THIS_CPU->active_address_space;
}

// TODO: Depracte this
ptable_t* vmm_get_active_pdir()
{
    return vmm_get_active_address_space()->pdir;
}

vm_address_space_t* vmm_get_kernel_address_space()
{
    return _vmm_kernel_address_space_ptr;
}

// TODO: Depracte this
ptable_t* vmm_get_kernel_pdir()
{
    return _vmm_kernel_pdir;
}

/**
 * PF HANDLER FUNCTIONS
 */

int vmm_tune_page_locked_impl(uintptr_t vaddr, mmu_flags_t mmu_flags)
{
    if (!THIS_CPU->active_address_space) {
        return -EACCES;
    }

    vaddr = ROUND_FLOOR(vaddr, VMM_PAGE_SIZE);
    if (vmm_is_copy_on_write(vaddr)) {
        return -EBUSY;
    }

    ptable_entity_t* page_desc = vm_get_entity(vaddr, PTABLE_LV0);
    if (vm_ptable_entity_is_present(page_desc, PTABLE_LV0)) {
        uintptr_t frame = vm_ptable_entity_get_frame(page_desc, PTABLE_LV0);
        vm_ptable_entity_set_default_flags(page_desc, PTABLE_LV0);
        vm_ptable_entity_set_mmu_flags(page_desc, PTABLE_LV0, mmu_flags);
        vm_ptable_entity_set_frame(page_desc, PTABLE_LV0, frame);
    } else {
        vmm_alloc_page_locked(vaddr, mmu_flags);
    }

    system_flush_local_tlb_entry(vaddr);
    return 0;
}

int vmm_alloc_page_no_fill_locked_impl(uintptr_t vaddr, mmu_flags_t mmu_flags)
{
    uintptr_t paddr = 0;
    do {
        paddr = vm_alloc_page_paddr();
        if (!paddr) {
            _vmm_sleep_locked();
        }
    } while (!paddr);

    int res = vmm_map_page_locked(vaddr, paddr, mmu_flags);
    return res;
}

/**
 * CPU BASED FUNCTIONS
 */

int vmm_switch_address_space_locked_impl(vm_address_space_t* vm_aspace)
{
    if (!vm_aspace) {
        return -EINVAL;
    }

    if (((uintptr_t)vm_aspace->pdir & (PTABLE_SIZE(PTABLE_LV_TOP) - 1)) != 0) {
        kpanic("vmm_switch_address_space_locked: not aligned pdir");
    }

    system_disable_interrupts();
    if (THIS_CPU->active_address_space == vm_aspace) {
        system_enable_interrupts();
        return 0;
    }
    THIS_CPU->active_address_space = vm_aspace;
    system_set_pdir((uintptr_t)_vmm_convert_vaddr2paddr((uintptr_t)vm_aspace->pdir));
    system_enable_interrupts();
    return 0;
}
