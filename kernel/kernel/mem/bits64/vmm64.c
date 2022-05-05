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
static ptable_t* _vmm_kernel_pdir1;
static spinlock_t _vmm_global_lock;
static kmemzone_t pspace_zone;

static bool _vmm_is_page_present(uintptr_t vaddr);

int vmm_swap_page(ptable_entity_t* page_desc, struct memzone* zone, uintptr_t vaddr) { return -1; }

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

static void vm_map_kernel_huge_page_1gb(uintptr_t vaddr, uintptr_t paddr, mmu_flags_t mmu_flags)
{
    // TODO(aarch64): Look like aarch64 and 4kb pages specific, need to unify this.
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
    _vmm_kernel_address_space.pdir0 = _vmm_kernel_pdir0;
    _vmm_kernel_address_space.pdir1 = _vmm_kernel_pdir1;
    spinlock_init(&_vmm_kernel_address_space.lock);

    memset((void*)_vmm_kernel_address_space.pdir0, 0, PTABLE_SIZE(PTABLE_LV_TOP));
    memset((void*)_vmm_kernel_address_space.pdir1, 0, PTABLE_SIZE(PTABLE_LV_TOP));

    // Set as an active one to set up kernel address space.
    THIS_CPU->active_address_space = _vmm_kernel_address_space_ptr;
}

static void vmm_create_kernel_ptables(boot_args_t* args)
{
    // TODO(aarch64): Replace constant(1024).
    vmm_map_pages_locked(KERNEL_BASE, args->paddr, 1024, MMU_FLAG_PERM_WRITE | MMU_FLAG_PERM_READ | MMU_FLAG_PERM_EXEC);
    vmm_map_pages_locked(args->paddr, args->paddr, 1024, MMU_FLAG_PERM_WRITE | MMU_FLAG_PERM_READ | MMU_FLAG_PERM_EXEC);
    vm_map_kernel_huge_page_1gb(KERNEL_PADDR_BASE, args->paddr, MMU_FLAG_PERM_WRITE | MMU_FLAG_PERM_READ | MMU_FLAG_PERM_EXEC);

    // Debug mapping, should be removed.
    if (!args->fb_boot_desc.vaddr) {
        vmm_map_page_locked(0x09000000, 0x09000000, MMU_FLAG_PERM_WRITE | MMU_FLAG_PERM_READ | MMU_FLAG_PERM_EXEC);
    } else {
        // Mapping 16mb.
        vmm_map_pages_locked((uintptr_t)args->fb_boot_desc.vaddr, (uintptr_t)args->fb_boot_desc.paddr, 256 * 16, MMU_FLAG_PERM_WRITE | MMU_FLAG_PERM_READ | MMU_FLAG_PERM_EXEC);
    }
}

int vmm_init_setup_finished = 0;
int vmm_setup(boot_args_t* args)
{
    log("setting up VMM");
    spinlock_init(&_vmm_global_lock);
    kmemzone_init();
    vm_alloc_kernel_pdir();
    vmm_create_kernel_ptables(args);
    vm_pspace_init(args);
    _vmm_init_switch_to_kernel_pdir();
    vmm_setup_paddr_mapped_location(args);
    kmemzone_init_stage2();
    kmalloc_init();

    // After kmalloc is set up, we can allocate dynarr for zones.
    dynarr_init_of_size(memzone_t, &_vmm_kernel_address_space.zones, 4);
    vmm_init_setup_finished = 1;
    return 0;
}

int vmm_setup_secondary_cpu() { return -1; }

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
    }

    if (lv == PTABLE_LV0) {
        return _vmm_map_page_locked_lv0(ptable, vaddr, paddr, mmu_flags, lv);
    }

    ptable_t* child_ptable = vm_get_table(vaddr, lv);
    ASSERT(child_ptable);

    return _vmm_map_page_locked_impl(child_ptable, vaddr, paddr, mmu_flags, lower_level(lv));
}

int vmm_map_page_locked(uintptr_t vaddr, uintptr_t paddr, mmu_flags_t mmu_flags)
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
 * @brief Maps a page specified with addresses.
 *
 * @param vaddr The virtual address to map.
 * @param paddr The physical address to map to.
 * @param mmu_flags Permission flags to map with.
 * @return Status of the operation.
 */
int vmm_map_page(uintptr_t vaddr, uintptr_t paddr, mmu_flags_t mmu_flags)
{
    vm_address_space_t* active_address_space = vmm_get_active_address_space();
    spinlock_acquire(&active_address_space->lock);
    int res = vmm_map_page_locked(vaddr, paddr, mmu_flags);
    spinlock_release(&active_address_space->lock);
    return res;
}

int vmm_map_pages_locked(uintptr_t vaddr, uintptr_t paddr, size_t n_pages, mmu_flags_t mmu_flags)
{
    vaddr = ROUND_FLOOR(vaddr, VMM_PAGE_SIZE);
    paddr = ROUND_FLOOR(paddr, VMM_PAGE_SIZE);

    int status = 0;
    for (; n_pages; paddr += VMM_PAGE_SIZE, vaddr += VMM_PAGE_SIZE, n_pages--) {
        if ((status = vmm_map_page_locked(vaddr, paddr, mmu_flags) != 0)) {
            return status;
        }
    }

    return 0;
}

/**
 * @brief Maps several pages specified with addresses and count.
 *
 * @param vaddr The virtual address to map.
 * @param paddr The physical address to map to.
 * @param n_pages Count of sequential pages to map.
 * @param mmu_flags Permission flags to map with.
 * @return Status of the operation.
 */
int vmm_map_pages(uintptr_t vaddr, uintptr_t paddr, size_t n_pages, mmu_flags_t mmu_flags)
{
    vm_address_space_t* active_address_space = vmm_get_active_address_space();
    spinlock_acquire(&active_address_space->lock);
    int res = vmm_map_pages_locked(vaddr, paddr, n_pages, mmu_flags);
    spinlock_release(&active_address_space->lock);
    return res;
}

/**
 * @brief Unmaps a page specified with addresses.
 *
 * @param vaddr The virtual address to unmap.
 * @return Status of the operation.
 */
int vmm_unmap_page_locked(uintptr_t vaddr)
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
 * @brief Unmaps a page specified with addresses.
 *
 * @param vaddr The virtual address to unmap.
 * @return Status of the operation.
 */
int vmm_unmap_page(uintptr_t vaddr)
{
    vm_address_space_t* active_address_space = vmm_get_active_address_space();
    spinlock_acquire(&active_address_space->lock);
    int res = vmm_unmap_page_locked(vaddr);
    spinlock_release(&active_address_space->lock);
    return res;
}

/**
 * @brief Unmaps several pages specified with addresses and count.
 *
 * @param vaddr The virtual address to unmap.
 * @param n_pages Count of sequential pages to unmap.
 * @return Status of the operation.
 */
int vmm_unmap_pages_locked(uintptr_t vaddr, size_t n_pages)
{
    vaddr = ROUND_FLOOR(vaddr, VMM_PAGE_SIZE);

    int status = 0;
    for (; n_pages; vaddr += VMM_PAGE_SIZE, n_pages--) {
        if ((status = vmm_unmap_page_locked(vaddr) < 0)) {
            return status;
        }
    }

    return 0;
}

/**
 * @brief Unmaps several pages specified with addresses and count.
 *
 * @param vaddr The virtual address to unmap.
 * @param n_pages Count of sequential pages to unmap.
 * @return Status of the operation.
 */
int vmm_unmap_pages(uintptr_t vaddr, size_t n_pages)
{
    vm_address_space_t* active_address_space = vmm_get_active_address_space();
    spinlock_acquire(&active_address_space->lock);
    int res = vmm_unmap_pages_locked(vaddr, n_pages);
    spinlock_release(&active_address_space->lock);
    return res;
}

static int vmm_alloc_page_no_fill_locked(uintptr_t vaddr, mmu_flags_t mmu_flags)
{
    uintptr_t paddr = vm_alloc_page_paddr();
    // TODO: Add sleep here.
    int res = vmm_map_page_locked(vaddr, paddr, mmu_flags);
    return res;
}

static int vmm_alloc_page_locked(uintptr_t vaddr, mmu_flags_t mmu_flags)
{
    int err = vmm_alloc_page_no_fill_locked(vaddr, mmu_flags);
    if (err) {
        return err;
    }

    void* dest = (void*)ROUND_FLOOR(vaddr, VMM_PAGE_SIZE);
    memset(dest, 0, VMM_PAGE_SIZE);
    return 0;
}

int vmm_alloc_page(uintptr_t vaddr, mmu_flags_t mmu_flags)
{
    vm_address_space_t* active_address_space = vmm_get_active_address_space();
    spinlock_acquire(&active_address_space->lock);
    if (_vmm_is_page_present(vaddr)) {
        spinlock_release(&active_address_space->lock);
        return -EALREADY;
    }

    int err = vmm_alloc_page_no_fill_locked(vaddr, mmu_flags);
    spinlock_release(&active_address_space->lock);
    if (err) {
        return err;
    }

    void* dest = (void*)ROUND_FLOOR(vaddr, VMM_PAGE_SIZE);
    memset(dest, 0, VMM_PAGE_SIZE);
    return 0;
}

int vmm_tune_page(uintptr_t vaddr, mmu_flags_t mmu_flags) { return -1; }
int vmm_tune_pages(uintptr_t vaddr, size_t length, mmu_flags_t mmu_flags) { return -1; }

vm_address_space_t* vmm_new_address_space() { return NULL; }
vm_address_space_t* vmm_new_forked_address_space() { return NULL; }
int vmm_free_address_space(vm_address_space_t* vm_aspace) { return -1; }

bool vmm_is_copy_on_write(uintptr_t vaddr) { return false; }

/**
 * @brief Checks if page is present.
 *
 * @param vaddr The virtual address to examine.
 * @return True if page is present.
 */
static bool _vmm_is_page_present(uintptr_t vaddr)
{
    ptable_entity_t* page_desc = vm_get_entity(vaddr, PTABLE_LV0);
    if (!page_desc) {
        return false;
    }
    return vm_ptable_entity_is_present(page_desc, PTABLE_LV0);
}

static memzone_t* _vmm_memzone_for_active_address_space(uintptr_t vaddr)
{
    return memzone_find(vmm_get_active_address_space(), vaddr);
}

static int vm_alloc_kernel_page_locked(uintptr_t vaddr)
{
    // A zone with standard mmu flags is allocated for kernel, while
    // we could use a approach similar to user pages, where mmu flags
    // are dependent on the zone.
    return vmm_alloc_page_locked(vaddr, MMU_FLAG_PERM_READ | MMU_FLAG_PERM_WRITE | MMU_FLAG_PERM_EXEC);
}

static int vm_alloc_user_page_no_fill_locked(memzone_t* zone, uintptr_t vaddr)
{
    if (!zone) {
        return -ESRCH;
    }
    return vmm_alloc_page_no_fill_locked(vaddr, zone->mmu_flags);
}

static int vm_alloc_user_page_locked(memzone_t* zone, uintptr_t vaddr)
{
    if (!zone) {
        return -EFAULT;
    }
    return vmm_alloc_page_locked(vaddr, zone->mmu_flags);
}

/**
 * @brief Allocate a new page for vaddr. Flags are dependent on memzone
 * associated with the virtual address.
 */
static int vm_alloc_page_with_perm(uintptr_t vaddr)
{
    if (IS_USER_VADDR(vaddr) && vmm_get_active_pdir() != vmm_get_kernel_pdir()) {
        return vm_alloc_user_page_locked(_vmm_memzone_for_active_address_space(vaddr), vaddr);
    }
    // Should keep lockless, since kernel interrupt could happen while setting VMM.
    return vm_alloc_kernel_page_locked(vaddr);
}

/**
 * @brief Prepare the page for writing. Might allocate a page if needed.
 */
static int _vmm_ensure_write_to_page(uintptr_t vaddr)
{
    // TODO(aarch64): CoW is not supported yet.
    if (!_vmm_is_page_present(vaddr)) {
        int err = vm_alloc_page_with_perm(vaddr);
        if (err) {
            return err;
        }
    }

    return 0;
}

/**
 * @brief Prepare the page for writing. Might allocate a page if needed.
 */
static int _vmm_ensure_write_to_range(uintptr_t vaddr, size_t length)
{
    uintptr_t page_addr = ROUND_FLOOR(vaddr, VMM_PAGE_SIZE);
    while (page_addr < vaddr + length) {
        int err = _vmm_ensure_write_to_page(page_addr);
        if (err) {
            return err;
        }
        page_addr += VMM_PAGE_SIZE;
    }
    return 0;
}

/**
 * @brief Ensures that writing to the address is safe and does NOT cause any faults.
 *        It might start resolving CoW if this is needed for writing.
 *
 * @param vaddr The virtual address to examine.
 * @param length The length of data which is written to the address.
 */
static ALWAYS_INLINE void vmm_ensure_writing_to_active_address_space_locked(uintptr_t dest_vaddr, size_t length)
{
    _vmm_ensure_write_to_range(dest_vaddr, length);
}

/**
 * @brief Ensures that writing to the address is safe and does NOT cause any faults.
 *        It might start resolving CoW if this is needed for writing.
 *
 * @param vaddr The virtual address to examine.
 * @param length The length of data which is written to the address.
 */
void vmm_ensure_writing_to_active_address_space(uintptr_t dest_vaddr, size_t length)
{
    vm_address_space_t* active_address_space = vmm_get_active_address_space();
    spinlock_acquire(&active_address_space->lock);
    vmm_ensure_writing_to_active_address_space_locked(dest_vaddr, length);
    spinlock_release(&active_address_space->lock);
}

void vmm_copy_to_address_space(vm_address_space_t* vm_aspace, void* src, uintptr_t dest_vaddr, size_t length) { }

vm_address_space_t* vmm_get_active_address_space() { return THIS_CPU->active_address_space; }
ptable_t* vmm_get_active_pdir() { return NULL; }
vm_address_space_t* vmm_get_kernel_address_space() { return _vmm_kernel_address_space_ptr; }
ptable_t* vmm_get_kernel_pdir() { return NULL; }

int vmm_switch_address_space_locked(vm_address_space_t* vm_aspace) { return -1; }
int vmm_switch_address_space(vm_address_space_t* vm_aspace) { return -1; }

int vmm_page_fault_handler(arch_pf_info_t info, uintptr_t vaddr) { return -1; }