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

/**
 * MEMZONE FUNCTIONS
 */

memzone_t* vmm_memzone_for_active_address_space(uintptr_t vaddr)
{
    return memzone_find(vmm_get_active_address_space(), vaddr);
}

/**
 * VMM MAP PAGES
 */

extern int vmm_map_page_locked_impl(uintptr_t vaddr, uintptr_t paddr, mmu_flags_t mmu_flags);
extern int vmm_unmap_page_locked_impl(uintptr_t vaddr);

/**
 * @brief Maps a page specified with addresses.
 *
 * @param vaddr The virtual address to map.
 * @param paddr The physical address to map to.
 * @param mmu_flags Permission flags to map with.
 * @return Status of the operation.
 */
int vmm_map_page_locked(uintptr_t vaddr, uintptr_t paddr, mmu_flags_t mmu_flags)
{
    return vmm_map_page_locked_impl(vaddr, paddr, mmu_flags);
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

/**
 * @brief Unmaps a page specified with addresses.
 *
 * @param vaddr The virtual address to unmap.
 * @return Status of the operation.
 */
int vmm_unmap_page_locked(uintptr_t vaddr)
{
    return vmm_unmap_page_locked_impl(vaddr);
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
 * @brief Maps several pages specified with addresses and count.
 *
 * @param vaddr The virtual address to map.
 * @param paddr The physical address to map to.
 * @param n_pages Count of sequential pages to map.
 * @param mmu_flags Permission flags to map with.
 * @return Status of the operation.
 */
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

/**
 * VMM TUNE PAGES
 */

extern int vmm_tune_page_locked_impl(uintptr_t vaddr, mmu_flags_t mmu_flags);

static int vmm_tune_page_locked(uintptr_t vaddr, mmu_flags_t mmu_flags)
{
    return vmm_tune_page_locked_impl(vaddr, mmu_flags);
}

int vmm_tune_page(uintptr_t vaddr, mmu_flags_t mmu_flags)
{
    vm_address_space_t* active_address_space = vmm_get_active_address_space();
    spinlock_acquire(&active_address_space->lock);
    int res = vmm_tune_page_locked(vaddr, mmu_flags);
    spinlock_release(&active_address_space->lock);
    return res;
}

static int vmm_tune_pages_locked(uintptr_t vaddr, size_t length, mmu_flags_t mmu_flags)
{
    uintptr_t page_addr = PAGE_START(vaddr);
    while (page_addr < vaddr + length) {
        vmm_tune_page_locked(page_addr, mmu_flags);
        page_addr += VMM_PAGE_SIZE;
    }
    return 0;
}

int vmm_tune_pages(uintptr_t vaddr, size_t length, mmu_flags_t mmu_flags)
{
    vm_address_space_t* active_address_space = vmm_get_active_address_space();
    spinlock_acquire(&active_address_space->lock);
    int res = vmm_tune_pages_locked(vaddr, length, mmu_flags);
    spinlock_release(&active_address_space->lock);
    return res;
}

/**
 * VMM ALLOC FUNCTIONS
 */

extern bool vmm_is_page_present_impl(uintptr_t vaddr);
extern int vmm_alloc_page_no_fill_locked_impl(uintptr_t vaddr, mmu_flags_t mmu_flags);

bool vmm_is_page_present(uintptr_t vaddr)
{
    return vmm_is_page_present_impl(vaddr);
}

static int vmm_alloc_page_no_fill_locked(uintptr_t vaddr, mmu_flags_t mmu_flags)
{
    return vmm_alloc_page_no_fill_locked_impl(vaddr, mmu_flags);
}

int vmm_alloc_page_locked(uintptr_t vaddr, mmu_flags_t mmu_flags)
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
    if (vmm_is_page_present(vaddr)) {
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

int vm_alloc_kernel_page_locked(uintptr_t vaddr)
{
    // A zone with standard mmu flags is allocated for kernel, while
    // we could use a approach similar to user pages, where mmu flags
    // are dependent on the zone.
    return vmm_alloc_page_locked(vaddr, MMU_FLAG_PERM_READ | MMU_FLAG_PERM_WRITE | MMU_FLAG_PERM_EXEC);
}

int vm_alloc_user_page_no_fill_locked(memzone_t* zone, uintptr_t vaddr)
{
    if (!zone) {
        return -ESRCH;
    }
    return vmm_alloc_page_no_fill_locked(vaddr, zone->mmu_flags);
}

int vm_alloc_user_page_locked(memzone_t* zone, uintptr_t vaddr)
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
int vm_alloc_page_with_perm(uintptr_t vaddr)
{
    if (IS_USER_VADDR(vaddr) && vmm_get_active_address_space() != vmm_get_kernel_address_space()) {
        return vm_alloc_user_page_locked(vmm_memzone_for_active_address_space(vaddr), vaddr);
    }
    // Should keep lockless, since kernel interrupt could happen while setting VMM.
    return vm_alloc_kernel_page_locked(vaddr);
}

/**
 * CoW FUNCTIONS
 */

extern int vmm_resolve_copy_on_write(uintptr_t vaddr);

/**
 * VMM CHECK FUNCTIONS
 */
bool vmm_is_page_swapped(uintptr_t vaddr);
int vmm_restore_swapped_page_locked(uintptr_t vaddr);

/**
 * @brief Loads an unpresent page. The funciton might create a new page or load
 *        an existing one from drive.
 *
 * @param vaddr The virtual address of a page to load.
 */
static int vmm_resolve_page_not_present_locked(uintptr_t vaddr)
{
    // CoW should be resolved before calling this function.
    assert(!vmm_is_copy_on_write(vaddr));

    if (IS_KERNEL_VADDR(vaddr)) {
        return vm_alloc_kernel_page_locked(vaddr);
    }

    memzone_t* zone = vmm_memzone_for_active_address_space(vaddr);
    if (!zone) {
        return -EFAULT;
    }

    if (vmm_is_page_swapped(vaddr)) {
        return vmm_restore_swapped_page_locked(vaddr);
    }

    int err = vm_alloc_user_page_no_fill_locked(zone, vaddr);
    if (err) {
        return err;
    }

    if (zone->ops && zone->ops->load_page_content) {
        return zone->ops->load_page_content(zone, vaddr);
    } else {
        void* dest = (void*)ROUND_FLOOR(vaddr, VMM_PAGE_SIZE);
        memset(dest, 0, VMM_PAGE_SIZE);
    }
    return 0;
}

static int _vmm_ensure_write_to_page_locked(uintptr_t vaddr)
{
    if (IS_USER_VADDR(vaddr) && vmm_is_copy_on_write(vaddr)) {
        int err = vmm_resolve_copy_on_write(vaddr);
        if (err) {
            return err;
        }
    }

    if (!vmm_is_page_present(vaddr)) {
        int err = vmm_resolve_page_not_present_locked(vaddr);
        if (err) {
            return err;
        }
    }

    return 0;
}

/**
 * @brief Prepare the page for writing. Might allocate a page if needed.
 */
static int _vmm_ensure_write_to_range_locked(uintptr_t vaddr, size_t length)
{
    uintptr_t page_addr = ROUND_FLOOR(vaddr, VMM_PAGE_SIZE);
    while (page_addr < vaddr + length) {
        int err = _vmm_ensure_write_to_page_locked(page_addr);
        if (err) {
            return err;
        }
        page_addr += VMM_PAGE_SIZE;
    }
    return 0;
}

static int _vmm_ensure_write_to_page(uintptr_t vaddr)
{
    vm_address_space_t* active_address_space = vmm_get_active_address_space();

    if (IS_USER_VADDR(vaddr) && vmm_is_copy_on_write(vaddr)) {
        spinlock_acquire(&active_address_space->lock);
        int err = vmm_resolve_copy_on_write(vaddr);
        spinlock_release(&active_address_space->lock);
        if (err) {
            return err;
        }
    }

    // Take lock only if page is not available.
    if (!vmm_is_page_present(vaddr)) {
        spinlock_acquire(&active_address_space->lock);
        int err = vmm_resolve_page_not_present_locked(vaddr);
        spinlock_release(&active_address_space->lock);
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
static inline void vmm_ensure_writing_to_active_address_space_locked(uintptr_t dest_vaddr, size_t length)
{
    _vmm_ensure_write_to_range_locked(dest_vaddr, length);
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
    _vmm_ensure_write_to_range(dest_vaddr, length);
}

static int _vmm_ensure_read_from_page_locked(uintptr_t vaddr)
{
    if (!vmm_is_page_present(vaddr)) {
        int err = vmm_resolve_page_not_present_locked(vaddr);
        if (err) {
            return err;
        }
    }

    return 0;
}

/**
 * @brief Prepare the page for writing. Might allocate a page if needed.
 */
static int _vmm_ensure_read_from_range_locked(uintptr_t vaddr, size_t length)
{
    uintptr_t page_addr = ROUND_FLOOR(vaddr, VMM_PAGE_SIZE);
    while (page_addr < vaddr + length) {
        int err = _vmm_ensure_read_from_page_locked(page_addr);
        if (err) {
            return err;
        }
        page_addr += VMM_PAGE_SIZE;
    }
    return 0;
}

static int _vmm_ensure_read_from_page(uintptr_t vaddr)
{
    vm_address_space_t* active_address_space = vmm_get_active_address_space();

    if (!vmm_is_page_present(vaddr)) {
        spinlock_acquire(&active_address_space->lock);
        int err = vmm_resolve_page_not_present_locked(vaddr);
        spinlock_release(&active_address_space->lock);
        if (err) {
            return err;
        }
    }
    return 0;
}

/**
 * @brief Prepare the page for reading. Might allocate a page if needed.
 */
static int _vmm_ensure_read_from_range(uintptr_t vaddr, size_t length)
{
    uintptr_t page_addr = ROUND_FLOOR(vaddr, VMM_PAGE_SIZE);
    while (page_addr < vaddr + length) {
        int err = _vmm_ensure_read_from_page(page_addr);
        if (err) {
            return err;
        }
        page_addr += VMM_PAGE_SIZE;
    }
    return 0;
}

/*
 * @brief Ensures that reading from the address is safe and does NOT cause any faults.
 *
 * @param vaddr The virtual address to examine.
 * @param length The length of data which is written to the address.
 */
void vmm_ensure_reading_from_active_address_space_locked(uintptr_t dest_vaddr, size_t length)
{
    _vmm_ensure_read_from_range_locked(dest_vaddr, length);
}

/*
 * @brief Ensures that reading from the address is safe and does NOT cause any faults.
 *
 * @param vaddr The virtual address to examine.
 * @param length The length of data which is written to the address.
 */
void vmm_ensure_reading_from_active_address_space(uintptr_t dest_vaddr, size_t length)
{
    _vmm_ensure_read_from_range(dest_vaddr, length);
}

/**
 * @brief Copies data to the kernel space if needed.
 *
 * @param data Pointer to the data.
 * @param length The length of data to copy.
 * @return Pointer to data in the kernel space.
 */
static void* _vmm_bring_to_kernel_space(void* data, size_t length)
{
    if ((uintptr_t)data >= KERNEL_BASE) {
        return data;
    }
    void* kaddr = kmalloc(length);
    _vmm_ensure_write_to_range_locked((uintptr_t)kaddr, length);
    memcpy(kaddr, data, length);
    return (void*)kaddr;
}

/**
 * SWAP FUNCTIONS
 */

extern bool vmm_is_page_swapped_impl(uintptr_t vaddr);
extern int vmm_restore_swapped_page_locked_impl(uintptr_t vaddr);
extern int vmm_swap_page_impl(ptable_entity_t* page_desc, memzone_t* zone, uintptr_t vaddr);

bool vmm_is_page_swapped(uintptr_t vaddr)
{
    return vmm_is_page_swapped_impl(vaddr);
}

int vmm_restore_swapped_page_locked(uintptr_t vaddr)
{
    return vmm_restore_swapped_page_locked_impl(vaddr);
}

int vmm_swap_page(ptable_entity_t* page_desc, memzone_t* zone, uintptr_t vaddr)
{
    return vmm_swap_page_impl(page_desc, zone, vaddr);
}

/**
 * ADDRESS SPACE FUNCTIONS
 */

extern vm_address_space_t* vmm_alloc_new_address_space_locked();
extern int vmm_fill_up_new_address_space(vm_address_space_t* new_aspace);
extern int vmm_fill_up_forked_address_space(vm_address_space_t* new_aspace);
extern int vmm_free_address_space_locked_impl(vm_address_space_t* vm_aspace);
extern int vmm_switch_address_space_locked_impl(vm_address_space_t* vm_aspace);

vm_address_space_t* vmm_new_address_space()
{
    vm_address_space_t* new_aspace = vmm_alloc_new_address_space_locked();
    vmm_fill_up_new_address_space(new_aspace);
    return new_aspace;
}

vm_address_space_t* vmm_new_forked_address_space()
{
    vm_address_space_t* new_aspace = vmm_alloc_new_address_space_locked();
    vmm_fill_up_forked_address_space(new_aspace);
    memzone_copy(new_aspace, THIS_CPU->active_address_space);
    return new_aspace;
}

/**
 * @brief Switches to vm_aspace address space.
 *
 * @param vm_aspace The addess space to switch to.
 * @return Status of the operation.
 */
int vmm_switch_address_space_locked(vm_address_space_t* vm_aspace)
{
    return vmm_switch_address_space_locked_impl(vm_aspace);
}

/**
 * @brief Switches to vm_aspace address space.
 *
 * @param vm_aspace The addess space to switch to.
 * @return Status of the operation.
 */
int vmm_switch_address_space(vm_address_space_t* vm_aspace)
{
    int res = vmm_switch_address_space_locked_impl(vm_aspace);
    return res;
}

/**
 * @brief Frees address space. If target vm_aspace is an active address space,
 *        after deletion kernel will be moved to kernel address space.
 *
 * @param vm_aspace The address space for deletion.
 * @return Status of the operation.
 */
static int _vmm_free_address_space_locked(vm_address_space_t* vm_aspace)
{
    ASSERT(vm_aspace->count == 0);

    if (!vm_aspace) {
        return -EINVAL;
    }

    if (vm_aspace == vmm_get_kernel_address_space()) {
        return -EACCES;
    }

    return vmm_free_address_space_locked_impl(vm_aspace);
}

/**
 * @brief Frees address space. If target vm_aspace is an active address space,
 *        after deletion kernel will be moved to kernel address space.
 *
 * @param vm_aspace The address space for deletion.
 * @return Status of the operation.
 */
int vmm_free_address_space(vm_address_space_t* vm_aspace)
{
    // Taking the lock before deleting to be sure that address space could be
    // cleaned.
    spinlock_acquire(&vm_aspace->lock);
    int res = _vmm_free_address_space_locked(vm_aspace);
    return res;
}

/**
 * @brief Copies data from the source address of the active address space to the
 *        destination virtual address of the target address space.
 *
 * @param vm_aspace The addess space to copy into.
 * @param src The data source.
 * @param dest_vaddr The destination virtual address
 * @param length The length of data to be copied.
 */
void vmm_copy_to_address_space(vm_address_space_t* vm_aspace, void* src, uintptr_t dest_vaddr, size_t length)
{
    vm_address_space_t* prev_aspace = vmm_get_active_address_space();

    // Copy data to the kernel space, if needed.
    void* ksrc = _vmm_bring_to_kernel_space(src, length);

    spinlock_acquire(&vm_aspace->lock);
    vmm_switch_address_space_locked(vm_aspace);
    vmm_ensure_writing_to_active_address_space_locked(dest_vaddr, length);
    spinlock_release(&vm_aspace->lock);

    void* dest = (void*)dest_vaddr;
    memcpy(dest, ksrc, length);

    if ((uintptr_t)src < KERNEL_BASE) {
        kfree(ksrc);
    }

    vmm_switch_address_space_locked(prev_aspace);
}

/**
 * PAGE FAULT FUNCTIONS
 */

static int _vmm_on_page_not_present_locked(uintptr_t vaddr)
{
    if (vmm_is_page_present(vaddr)) {
        return 0;
    }

    // Resolving a potential CoW only for user pages.
    if (IS_USER_VADDR(vaddr) && vmm_is_copy_on_write(vaddr)) {
        int err = vmm_resolve_copy_on_write(vaddr);
        if (err) {
            return err;
        }
    }

    return vmm_resolve_page_not_present_locked(vaddr);
}

static int _vmm_pf_on_writing_locked(uintptr_t vaddr)
{
    int visited = 0;

    if (vmm_is_copy_on_write(vaddr)) {
        int err = vmm_resolve_copy_on_write(vaddr);
        if (err) {
            return err;
        }
        visited++;
    }

    if (!visited) {
        return -EFAULT;
    }

    return 0;
}

int vmm_page_fault_handler(arch_pf_info_t info, uintptr_t vaddr)
{
    vm_address_space_t* active_address_space = vmm_get_active_address_space();
    mmu_pf_info_flags_t pf_info_flags = vm_arch_parse_pf_info(info);

    if (TEST_FLAG(pf_info_flags, MMU_PF_INFO_ON_NOT_PRESENT)) {
        spinlock_acquire(&active_address_space->lock);
        int res = _vmm_on_page_not_present_locked(vaddr);
        spinlock_release(&active_address_space->lock);
        return res;
    }

    if (TEST_FLAG(pf_info_flags, MMU_PF_INFO_ON_WRITE)) {
        spinlock_acquire(&active_address_space->lock);
        int res = _vmm_pf_on_writing_locked(vaddr);
        spinlock_release(&active_address_space->lock);
        return res;
    }

    return -EINVAL;
}

static void dump_pf_info(arch_pf_info_t info, uintptr_t vaddr)
{
    mmu_pf_info_flags_t pf_info_flags = vm_arch_parse_pf_info(info);
    ptable_entity_t* ent = vm_get_entity(vaddr, PTABLE_LV0);
    log("PF %zx mmu flags: %zx, arch flags: %zx", vaddr, pf_info_flags, info);
    if (ent) {
        log("ent %zx", *ent);
    } else {
        log("No ent");
    }
}