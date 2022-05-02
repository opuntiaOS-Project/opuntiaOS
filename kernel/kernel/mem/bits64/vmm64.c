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

// #define VMM_DEBUG
// #define VMM_DEBUG_SWAP

static vm_address_space_t _vmm_kernel_address_space;
static vm_address_space_t* _vmm_kernel_address_space_ptr = &_vmm_kernel_address_space;
static uintptr_t _vmm_kernel_pdir_paddr;
static ptable_t* _vmm_kernel_pdir;
static spinlock_t _vmm_global_lock;
static kmemzone_t pspace_zone;
uintptr_t kernel_ptables_start_paddr = 0x0;

static void vm_alloc_kernel_pdir()
{
    _vmm_kernel_pdir_paddr = (uintptr_t)pmm_alloc_aligned(PTABLE_SIZE(PTABLE_LV_TOP), PTABLE_SIZE(PTABLE_LV_TOP));
    _vmm_kernel_pdir = (ptable_t*)(_vmm_kernel_pdir_paddr + pmm_get_state()->boot_args->vaddr - pmm_get_state()->boot_args->paddr);
    _vmm_kernel_address_space.count = 1;
    _vmm_kernel_address_space.pdir = _vmm_kernel_pdir;
    spinlock_init(&_vmm_kernel_address_space.lock);
    memset((void*)_vmm_kernel_address_space.pdir, 0, PTABLE_SIZE(PTABLE_LV_TOP));

    log("allocated %x", PTABLE_SIZE(PTABLE_LV_TOP));

    // Set as an active one to set up kernel address space.
    THIS_CPU->active_address_space = _vmm_kernel_address_space_ptr;
}

static void vmm_create_kernel_ptables()
{
    // We create only the most top level kernel ptables which goes after the address ifself.
    const ptable_lv_t ptable_entity_lv = PTABLE_LV_TOP;
    const size_t kernel_address_start = VM_VADDR_OFFSET_AT_LEVEL(KERNEL_BASE, ptable_entity_lv);
    const size_t table_coverage = (1ull << ptable_entity_vaddr_offset_at_level[ptable_entity_lv]);
    uintptr_t kernel_ptables_vaddr = KERNEL_BASE;

    log("Kernel start from %llx %llx of %llx", KERNEL_BASE, kernel_address_start, PTABLE_ENTITY_COUNT(ptable_entity_lv));
    const size_t need_pages_for_kernel_ptables = PTABLE_ENTITY_COUNT(ptable_entity_lv) - kernel_address_start;
    uintptr_t ptables_paddr = (uintptr_t)pmm_alloc_aligned(need_pages_for_kernel_ptables * VMM_PAGE_SIZE, VMM_PAGE_SIZE);
    if (!ptables_paddr) {
        kpanic("_vmm_create_kernel_ptables: No free space in pmm to alloc kernel ptables");
    }
    log("Need ptables for kernel shit %llx, with coverage %llx", need_pages_for_kernel_ptables, table_coverage);
    kernel_ptables_start_paddr = ptables_paddr;

    uintptr_t ptable_paddr = ptables_paddr;
    for (int i = kernel_address_start; i < PTABLE_ENTITY_COUNT(ptable_entity_lv); i++) {
        ptable_entity_t* ptable_entity = vm_get_entity(kernel_ptables_vaddr, ptable_entity_lv);
        log("Looking for %p", ptable_entity);
        vm_ptable_entity_set_default_flags(ptable_entity, ptable_entity_lv);
        vm_ptable_entity_set_mmu_flags(ptable_entity, ptable_entity_lv, MMU_FLAG_PERM_READ | MMU_FLAG_PERM_WRITE);
        vm_ptable_entity_set_frame(ptable_entity, ptable_entity_lv, ptable_paddr);

        ptable_paddr += PTABLE_SIZE(lower_level(ptable_entity_lv));
        kernel_ptables_vaddr += table_coverage;
    }
}

int vmm_setup()
{
    log("setting up VMM");
    spinlock_init(&_vmm_global_lock);
    kmemzone_init();
    vm_alloc_kernel_pdir();
    vmm_create_kernel_ptables();
    return 0;
}

int vmm_setup_secondary_cpu() { return -1; }

int vmm_free_address_space(vm_address_space_t* vm_aspace) { return -1; }

int vmm_alloc_page(uintptr_t vaddr, mmu_flags_t mmu_flags) { return -1; }
int vmm_tune_page(uintptr_t vaddr, mmu_flags_t mmu_flags) { return -1; }
int vmm_tune_pages(uintptr_t vaddr, size_t length, mmu_flags_t mmu_flags) { return -1; }

int vmm_map_page(uintptr_t vaddr, uintptr_t paddr, mmu_flags_t mmu_flags) { return -1; }
int vmm_map_pages(uintptr_t vaddr, uintptr_t paddr, size_t n_pages, mmu_flags_t mmu_flags) { return -1; }
int vmm_unmap_page(uintptr_t vaddr) { return -1; }
int vmm_unmap_pages(uintptr_t vaddr, size_t n_pages) { return -1; }
int vmm_copy_page(uintptr_t to_vaddr, uintptr_t src_vaddr, ptable_t* src_ptable) { return -1; }
int vmm_swap_page(ptable_entity_t* page_desc, struct memzone* zone, uintptr_t vaddr) { return -1; }

int vmm_map_page_locked(uintptr_t vaddr, uintptr_t paddr, mmu_flags_t mmu_flags) { return -1; }
int vmm_map_pages_locked(uintptr_t vaddr, uintptr_t paddr, size_t n_pages, mmu_flags_t mmu_flags) { return -1; }
int vmm_unmap_page_locked(uintptr_t vaddr) { return -1; }
int vmm_unmap_pages_locked(uintptr_t vaddr, size_t n_pages) { return -1; }

vm_address_space_t* vmm_new_address_space() { return NULL; }
vm_address_space_t* vmm_new_forked_address_space() { return NULL; }

bool vmm_is_copy_on_write(uintptr_t vaddr) { return false; }

void vmm_ensure_writing_to_active_address_space(uintptr_t dest_vaddr, size_t length) { }
void vmm_copy_to_address_space(vm_address_space_t* vm_aspace, void* src, uintptr_t dest_vaddr, size_t length) { }

vm_address_space_t* vmm_get_active_address_space() { return NULL; }
ptable_t* vmm_get_active_pdir() { return NULL; }
vm_address_space_t* vmm_get_kernel_address_space() { return NULL; }
ptable_t* vmm_get_kernel_pdir() { return NULL; }

int vmm_switch_address_space_locked(vm_address_space_t* vm_aspace) { return -1; }
int vmm_switch_address_space(vm_address_space_t* vm_aspace) { return -1; }

int vmm_page_fault_handler(arch_pf_info_t info, uintptr_t vaddr) { return -1; }