/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#ifndef __oneOS__MEM__VMM__VMM_H
#define __oneOS__MEM__VMM__VMM_H

#include <types.h>
#include <mem/pmm.h>
#include <mem/vmm/pte.h>
#include <mem/vmm/pde.h>

#define KB (1024)
#define MB (1024 * 1024)

#define VMM_PTE_COUNT (1024)
#define VMM_PDE_COUNT (1024)
#define VMM_PAGE_SIZE (4096)

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


/**
 * PUBLIC FUNCTIONS
 */

int vmm_setup();

table_desc_t* vmm_pdirectory_lookup(pdirectory_t *t_pdir, uint32_t t_addr);
page_desc_t* vmm_ptable_lookup(ptable_t *t_ptable, uint32_t t_addr);
int vmm_allocate_ptable(uint32_t vaddr);
int vmm_free_pdir(pdirectory_t* pdir);

int vmm_map_page(uint32_t vaddr, uint32_t paddr, bool owner);
int vmm_map_pages(uint32_t vaddr, uint32_t paddr, uint32_t n_pages, bool owner);
int vmm_unmap_page(uint32_t vaddr);
int vmm_copy_page(uint32_t vaddr, ptable_t *src_ptable);

pdirectory_t* vmm_new_user_pdir();
pdirectory_t* vmm_new_forked_user_pdir();
void vmm_copy_program_data(pdirectory_t* dir, uint8_t* data, uint32_t data_size); // will be deprecated
void* vmm_bring_to_kernel(uint8_t* src, uint32_t length);
void vmm_copy_to_pdir(pdirectory_t* pdir, uint8_t* src, uint32_t dest_vaddr, uint32_t length);
void vmm_zero_user_pages(pdirectory_t* pdir);
pdirectory_t* vmm_get_active_pdir();

int vmm_load_page(uint32_t vaddr);
int vmm_alloc_page(page_desc_t* page);
int vmm_free_page(page_desc_t* page);
void vmm_page_fault_handler(uint8_t info, uint32_t vaddr);

int vmm_switch_pdir(pdirectory_t *pdir);
void vmm_enable_paging();
void vmm_disable_paging();

#endif // __oneOS__MEM__VMM__VMM_H
