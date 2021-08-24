/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_MEM_VMM_VMM_H
#define _KERNEL_MEM_VMM_VMM_H

#include <libkern/libkern.h>
#include <mem/pmm.h>
#include <platform/generic/vmm/consts.h>
#include <platform/generic/vmm/pde.h>
#include <platform/generic/vmm/pte.h>

#define vmm_is_kernel_address(add) (add >= KERNEL_BASE)

/* Note: If you change them, change also proc zone flags */
enum PAGE_FLAGS {
    PAGE_WRITABLE = 0x1,
    PAGE_READABLE = 0x2,
    PAGE_EXECUTABLE = 0x4,
    PAGE_NOT_CACHEABLE = 0x8,
    PAGE_COW = 0x10,
    PAGE_USER = 0x20,
};

#define USER_PAGE true
#define KERNEL_PAGE false
#define PAGE_CHOOSE_OWNER(vaddr) (vaddr >= KERNEL_BASE ? 0 : PAGE_USER)

enum VMM_ERR_CODES {
    VMM_ERR_PDIR,
    VMM_ERR_PTABLE,
    VMM_ERR_NO_SPACE,
    VMM_ERR_BAD_ADDR,
};

typedef struct {
    page_desc_t entities[VMM_PTE_COUNT];
} ptable_t;

typedef struct pdirectory {
    table_desc_t entities[VMM_PDE_COUNT];
} pdirectory_t;

enum VMM_PF_HANDLER {
    OK = 0,
    SHOULD_CRASH = -1,
};

struct dynamic_array;

/**
 * PUBLIC FUNCTIONS
 */

int vmm_setup();
int vmm_setup_secondary_cpu();

int vmm_allocate_ptable(uint32_t vaddr);
int vmm_free_ptable(uint32_t vaddr, struct dynamic_array* zones);
int vmm_free_pdir(pdirectory_t* pdir, struct dynamic_array* zones);

int vmm_map_page(uint32_t vaddr, uint32_t paddr, uint32_t settings);
int vmm_map_pages(uint32_t vaddr, uint32_t paddr, uint32_t n_pages, uint32_t settings);
int vmm_unmap_page(uint32_t vaddr);
int vmm_unmap_pages(uint32_t vaddr, uint32_t n_pages);
int vmm_copy_page(uint32_t to_vaddr, uint32_t src_vaddr, ptable_t* src_ptable);

pdirectory_t* vmm_new_user_pdir();
pdirectory_t* vmm_new_forked_user_pdir();
void* vmm_bring_to_kernel(uint8_t* src, uint32_t length);
void vmm_prepare_active_pdir_for_copying_at(uint32_t dest_vaddr, uint32_t length);
void vmm_copy_to_user(void* dest, void* src, uint32_t length);
void vmm_copy_to_pdir(pdirectory_t* pdir, void* src, uint32_t dest_vaddr, uint32_t length);
void vmm_zero_user_pages(pdirectory_t* pdir);

pdirectory_t* vmm_get_active_pdir();
pdirectory_t* vmm_get_kernel_pdir();

int vmm_load_page(uint32_t vaddr, uint32_t settings);
int vmm_tune_page(uint32_t vaddr, uint32_t settings);
int vmm_tune_pages(uint32_t vaddr, uint32_t length, uint32_t settings);
int vmm_free_page(uint32_t vaddr, page_desc_t* page, struct dynamic_array* zones);

int vmm_switch_pdir(pdirectory_t* pdir);
void vmm_enable_paging();
void vmm_disable_paging();

int vmm_page_fault_handler(uint32_t info, uint32_t vaddr);

#endif // _KERNEL_MEM_VMM_VMM_H
