/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libkern/bits/errno.h>
#include <libkern/kasan.h>
#include <libkern/libkern.h>
#include <libkern/lock.h>
#include <libkern/log.h>
#include <mem/kmalloc.h>
#include <mem/kmemzone.h>
#include <mem/memzone.h>
#include <mem/swapfile.h>
#include <mem/vm_alloc.h>
#include <mem/vm_pspace.h>
#include <mem/vmm.h>
#include <platform/generic/cpu.h>
#include <platform/generic/system.h>

// #define VMM_DEBUG
// #define VMM_DEBUG_SWAP

static vm_address_space_t _vmm_kernel_address_space;
static vm_address_space_t* _vmm_kernel_address_space_ptr = &_vmm_kernel_address_space;
static uintptr_t _vmm_kernel_pdir0_paddr;
static ptable_t* _vmm_kernel_pdir0;
static uintptr_t _vmm_kernel_pdir1_paddr;
ptable_t* _vmm_kernel_pdir1;
static spinlock_t _vmm_global_lock;
static kmemzone_t pspace_zone;

static bool _vmm_is_page_present(uintptr_t vaddr);

static int _vmm_init_switch_to_kernel_pdir()
{
    THIS_CPU->active_address_space = _vmm_kernel_address_space_ptr;
    system_disable_interrupts();
    system_set_pdir(_vmm_kernel_pdir0_paddr, _vmm_kernel_pdir1_paddr);
    system_enable_interrupts();
    return 0;
}

static void vmm_setup_paddr_mapped_location(boot_args_t* args)
{
    // Align it to 1gb, as it mapped with 1gb huge page it.
    uintptr_t vaddr = ROUND_FLOOR(KERNEL_PADDR_BASE, 1 << 30);
    uintptr_t paddr = ROUND_FLOOR(args->paddr, 1 << 30);

    extern uintptr_t vm_pspace_paddr_zone_offset;
    vm_pspace_paddr_zone_offset = vaddr - paddr;
}

static void vmm_setup_kasan()
{
#ifdef KASAN_ENABLED
    const size_t kasan_size = KERNEL_KASAN_SIZE;
    uintptr_t kasan_paddr = (uintptr_t)pmm_alloc(kasan_size);
    vmm_map_pages(KERNEL_KASAN_BASE, kasan_paddr, kasan_size / VMM_PAGE_SIZE, MMU_FLAG_PERM_WRITE | MMU_FLAG_PERM_READ);

    kasan_init(KERNEL_KASAN_BASE, KERNEL_KASAN_SIZE);
#endif
}

static void vm_map_kernel_huge_page_1gb(uintptr_t vaddr, uintptr_t paddr, mmu_flags_t mmu_flags)
{
    // TODO(aarch64): Look like aarch64 and 4kb pages specific, need to
    //                unify this for other page sizes as well.
    ASSERT(PTABLE_LV_TOP == PTABLE_LV2);
    vaddr = ROUND_FLOOR(vaddr, 1 << 30);
    paddr = ROUND_FLOOR(paddr, 1 << 30);

    ptable_entity_t* ptable_desc = vm_get_entity(vaddr, PTABLE_LV2);
    vm_ptable_entity_set_default_flags(ptable_desc, PTABLE_LV2);
    vm_ptable_entity_set_mmu_flags(ptable_desc, PTABLE_LV2, MMU_FLAG_HUGE_PAGE | mmu_flags);
    vm_ptable_entity_set_frame(ptable_desc, PTABLE_LV2, paddr);
}

static void vm_alloc_kernel_pdir()
{
    _vmm_kernel_pdir0_paddr = (uintptr_t)pmm_alloc_aligned(PTABLE_SIZE(PTABLE_LV_TOP), PTABLE_SIZE(PTABLE_LV_TOP));
    _vmm_kernel_pdir0 = (ptable_t*)(_vmm_kernel_pdir0_paddr - pmm_get_state()->boot_args->paddr + pmm_get_state()->boot_args->vaddr);

    _vmm_kernel_pdir1_paddr = (uintptr_t)pmm_alloc_aligned(PTABLE_SIZE(PTABLE_LV_TOP), PTABLE_SIZE(PTABLE_LV_TOP));
    _vmm_kernel_pdir1 = (ptable_t*)(_vmm_kernel_pdir1_paddr - pmm_get_state()->boot_args->paddr + pmm_get_state()->boot_args->vaddr);

    _vmm_kernel_address_space.count = 1;
    _vmm_kernel_address_space.pdir = _vmm_kernel_pdir0;
    spinlock_init(&_vmm_kernel_address_space.lock);

    memset((void*)_vmm_kernel_pdir0, 0, PTABLE_SIZE(PTABLE_LV_TOP));
    memset((void*)_vmm_kernel_pdir1, 0, PTABLE_SIZE(PTABLE_LV_TOP));

    // Set as an active one to set up kernel address space.
    THIS_CPU->active_address_space = _vmm_kernel_address_space_ptr;
}

static void vmm_create_kernel_ptables(boot_args_t* args)
{
    const pmm_state_t* pmm_state = pmm_get_state();
    uintptr_t paddr = pmm_state->boot_args->paddr;
    uintptr_t vaddr = pmm_state->kernel_va_base;
    uintptr_t end_vaddr = pmm_state->kernel_va_base + pmm_state->kernel_data_size;

    // Mapping kernel and MAT.
    while (vaddr < end_vaddr) {
        vmm_map_page_locked(vaddr, paddr, MMU_FLAG_PERM_WRITE | MMU_FLAG_PERM_READ | MMU_FLAG_PERM_EXEC);
        // vmm_map_page_locked(paddr, paddr, MMU_FLAG_PERM_WRITE | MMU_FLAG_PERM_READ | MMU_FLAG_PERM_EXEC);
        vaddr += VMM_PAGE_SIZE;
        paddr += VMM_PAGE_SIZE;
    }

    // Mapping kernel ptable to access them.
    vmm_map_page_locked((uintptr_t)_vmm_kernel_pdir0, _vmm_kernel_pdir0_paddr, MMU_FLAG_PERM_WRITE | MMU_FLAG_PERM_READ | MMU_FLAG_UNCACHED);
    vmm_map_page_locked((uintptr_t)_vmm_kernel_pdir1, _vmm_kernel_pdir1_paddr, MMU_FLAG_PERM_WRITE | MMU_FLAG_PERM_READ | MMU_FLAG_UNCACHED);

    // Mapping physical RAM to access it inside kernelspace.
    // Note: This area is marked as uncached, since VIVT caches could break translation,
    // since code could change content of a transation table and changes will stuck
    // in VIVT caches and could be not evicted in some scenarios.
    vm_map_kernel_huge_page_1gb(KERNEL_PADDR_BASE, args->paddr, MMU_FLAG_PERM_WRITE | MMU_FLAG_PERM_READ | MMU_FLAG_UNCACHED);

    // Debug mapping, should be removed.
    if (!args->fb_boot_desc.vaddr) {
        vmm_map_page_locked(0x09000000, 0x09000000, MMU_FLAG_PERM_WRITE | MMU_FLAG_PERM_READ);
    } else {
        // Mapping 16mb.
        vmm_map_pages_locked((uintptr_t)args->fb_boot_desc.vaddr, (uintptr_t)args->fb_boot_desc.paddr, 256 * 16, MMU_FLAG_PERM_WRITE | MMU_FLAG_PERM_READ);
    }
}

int vmm_init_setup_finished = 0;
int vmm_setup(boot_args_t* args)
{
    spinlock_init(&_vmm_global_lock);
    kmemzone_init();
    vm_alloc_kernel_pdir();
    vmm_create_kernel_ptables(args);
    vm_pspace_init(args);
    _vmm_init_switch_to_kernel_pdir();
    vmm_setup_paddr_mapped_location(args);
    vmm_setup_kasan();
    kmemzone_init_stage2();
    kmalloc_init();

    // After kmalloc is set up, we can allocate dynarr for zones.
    dynarr_init_of_size(memzone_t, &_vmm_kernel_address_space.zones, 4);
    vmm_init_setup_finished = 1;
    return 0;
}

int vmm_setup_secondary_cpu() { return -1; }

/**
 * VMM MAP PAGES
 */

static int _vmm_map_page_locked_lv0(ptable_t* ptable, uintptr_t vaddr, uintptr_t paddr, mmu_flags_t mmu_flags, ptable_lv_t lv)
{
    if (!ptable) {
        return -EINVAL;
    }

    ptable_entity_t* page_desc = vm_get_entity(vaddr, PTABLE_LV0);
    vm_ptable_entity_set_default_flags(page_desc, PTABLE_LV0);
    vm_ptable_entity_set_mmu_flags(page_desc, PTABLE_LV0, mmu_flags | MMU_FLAG_PERM_READ);
    vm_ptable_entity_set_frame(page_desc, PTABLE_LV0, paddr);

#ifdef VMM_DEBUG
    log("Page mapped %zx at %zx :: (%p) => %llx", vaddr, paddr, page_desc, *page_desc);
#endif

    system_flush_local_tlb_entry(vaddr);
    return 0;
}

static int _vmm_map_page_locked_impl(ptable_t* ptable, uintptr_t vaddr, uintptr_t paddr, mmu_flags_t mmu_flags, ptable_lv_t lv)
{
    if (!ptable) {
        return -EINVAL;
    }

    ptable_lv_t upper_lv = upper_level(lv);
    ptable_entity_t* ptable_desc = vm_get_entity(vaddr, upper_lv);
    if (!vm_ptable_entity_is_present(ptable_desc, upper_lv)) {
        uintptr_t ptable_paddr = vm_alloc_ptable_paddr(lv);
        vm_ptable_entity_set_default_flags(ptable_desc, upper_lv);
        vm_ptable_entity_set_mmu_flags(ptable_desc, upper_lv, MMU_FLAG_PERM_READ | MMU_FLAG_PERM_WRITE | MMU_FLAG_PERM_EXEC | MMU_FLAG_NONPRIV);
        vm_ptable_entity_set_frame(ptable_desc, upper_lv, ptable_paddr);
#ifdef VMM_DEBUG
        log("Table mapped[%d] %zx at %zx :: (%p) => %llx", lv, vaddr, ptable_paddr, ptable_desc, *ptable_desc);
#endif
        ptable_t* this_table = vm_get_table(vaddr, lv);
        memset(this_table, 0, PTABLE_SIZE(lv));
    }

    if (lv == PTABLE_LV0) {
        return _vmm_map_page_locked_lv0(ptable, vaddr, paddr, mmu_flags, lv);
    }

    ptable_t* child_ptable = vm_get_table(vaddr, lv);
    ASSERT(child_ptable);

    return _vmm_map_page_locked_impl(child_ptable, vaddr, paddr, mmu_flags, lower_level(lv));
}

int vmm_map_page_locked_impl(uintptr_t vaddr, uintptr_t paddr, mmu_flags_t mmu_flags)
{
    ptable_lv_t lv = PTABLE_LV_TOP;
    if (!THIS_CPU->active_address_space) {
        return -EACCES;
    }

    ptable_t* ptable = vm_get_table(vaddr, lv);
    if (!ptable) {
        return -EACCES;
    }

    return _vmm_map_page_locked_impl(ptable, vaddr, paddr, mmu_flags, lower_level(lv));
}

/**
 * @brief Unmaps a page specified with addresses.
 *
 * @param vaddr The virtual address to unmap.
 * @return Status of the operation.
 */
int vmm_unmap_page_locked_impl(uintptr_t vaddr)
{
    // TODO(aarch64): This does not clean up used table.
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
 * VMM TUNE PAGES
 */

extern int vmm_alloc_page_locked(uintptr_t vaddr, mmu_flags_t mmu_flags);

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

/**
 * VMM ALLOC FUNCTIONS
 */

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

int vmm_alloc_page_no_fill_locked_impl(uintptr_t vaddr, mmu_flags_t mmu_flags)
{
    uintptr_t paddr = vm_alloc_page_paddr();
    // TODO(aarch64): Add sleep here.
    int res = vmm_map_page_locked(vaddr, paddr, mmu_flags);
    return res;
}

/**
 * CoW FUNCTIONS
 */

bool vmm_is_copy_on_write(uintptr_t vaddr) { return false; }

/**
 * @brief Resolves CoW for an active address space if needed.
 */
int vmm_resolve_copy_on_write(uintptr_t vaddr)
{
    return 0;
}

/**
 * SWAP FUNCTIONS
 */

bool vmm_is_page_swapped_impl(uintptr_t vaddr)
{
    return false;
}

int vmm_restore_swapped_page_locked_impl(uintptr_t vaddr)
{
    return -1;
}

int vmm_swap_page_impl(ptable_entity_t* page_desc, memzone_t* zone, uintptr_t vaddr)
{
    return -1;
}

/**
 * ADDRESS SPACE FUNCTIONS
 */

vm_address_space_t* vmm_alloc_new_address_space_locked()
{
    vm_address_space_t* new_address_space = vm_address_space_alloc();

    // TODO(aarch64): Add sleep here.
    ptable_t* new_pdir = vm_alloc_ptable_lv_top();
    new_address_space->pdir = new_pdir;
    return new_address_space;
}

int vmm_fill_up_new_address_space(vm_address_space_t* new_aspace)
{
    memset(new_aspace->pdir, 0, PTABLE_SIZE(PTABLE_LV_TOP));
    return 0;
}

static int _vmm_copy_of_aspace(ptable_t* old, ptable_t* new, ptable_lv_t lv)
{
    extern void* paddr_to_vaddr(uintptr_t paddr);
    if (lv == PTABLE_LV0) {
        for (int i = 0; i < PTABLE_ENTITY_COUNT(lv); i++) {
            if (vm_ptable_entity_is_present(&old->entities[i], lv)) {
                uintptr_t old_page_paddr = vm_ptable_entity_get_frame(&old->entities[i], lv);
                uintptr_t new_child_page_paddr = vm_alloc_page_paddr();

                memcpy(paddr_to_vaddr(new_child_page_paddr), paddr_to_vaddr(old_page_paddr), VMM_PAGE_SIZE);

                new->entities[i] = old->entities[i];
                vm_ptable_entity_set_frame(&new->entities[i], lv, new_child_page_paddr);

#ifdef VMM_DEBUG
                log("Copy page %zx to %zx", old_page_paddr, new_child_page_paddr);
#endif
            } else {
                vm_ptable_entity_invalidate(&new->entities[i], lv);
            }
        }
    } else {
        ptable_lv_t lowerlv = lower_level(lv);
        for (int i = 0; i < PTABLE_ENTITY_COUNT(lv); i++) {
            if (vm_ptable_entity_is_present(&old->entities[i], lv)) {
                uintptr_t old_ptable_paddr = vm_ptable_entity_get_frame(&old->entities[i], lv);
                uintptr_t new_child_ptable_paddr = vm_alloc_ptable_paddr(lowerlv);

                new->entities[i] = old->entities[i];
                vm_ptable_entity_set_frame(&new->entities[i], lv, new_child_ptable_paddr);

#ifdef VMM_DEBUG
                log("Copy table [%d] %zx to %zx", lv, old_ptable_paddr, new_child_ptable_paddr);
#endif
                _vmm_copy_of_aspace(paddr_to_vaddr(old_ptable_paddr), paddr_to_vaddr(new_child_ptable_paddr), lowerlv);
            } else {
                vm_ptable_entity_invalidate(&new->entities[i], lv);
            }
        }
    }

    return 0;
}

int vmm_fill_up_forked_address_space(vm_address_space_t* new_aspace)
{
    vm_address_space_t* active_address_space = THIS_CPU->active_address_space;
    spinlock_acquire(&active_address_space->lock);

    // TODO(aarch64): Implement CoW.
    _vmm_copy_of_aspace(active_address_space->pdir, new_aspace->pdir, PTABLE_LV_TOP);
    system_flush_whole_tlb();
    spinlock_release(&active_address_space->lock);
    return 0;
}

int vmm_free_address_space_locked_impl(vm_address_space_t* vm_aspace)
{
    return vm_pspace_free_address_space_locked(vm_aspace);
}

vm_address_space_t* vmm_get_active_address_space() { return THIS_CPU->active_address_space; }
vm_address_space_t* vmm_get_kernel_address_space() { return _vmm_kernel_address_space_ptr; }

static uintptr_t _vmm_convert_vaddr2paddr(uintptr_t vaddr)
{
    ptable_entity_t* page_desc = vm_get_entity(vaddr, PTABLE_LV0);
    return ((vm_ptable_entity_get_frame(page_desc, PTABLE_LV0)) | (vaddr & 0xfff));
}

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
    system_set_pdir(_vmm_convert_vaddr2paddr((uintptr_t)vm_aspace->pdir), _vmm_kernel_pdir1_paddr);
    system_flush_whole_tlb();
    system_enable_interrupts();
    return 0;
}
