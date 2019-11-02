#ifndef __oneOS__MEM__VMM__VMM_H
#define __oneOS__MEM__VMM__VMM_H

#include <types.h>
#include <mem/pmm.h>
#include <mem/vmm/pte.h>
#include <mem/vmm/pde.h>

#define VMM_PTE_COUNT (1024)
#define VMM_PDE_COUNT (1024)
#define VMM_PAGE_SIZE (4096)

typedef struct {
    pte_t entities[VMM_PTE_COUNT];
} ptable_t;

typedef struct pdirectory {
    pde_t entities[VMM_PDE_COUNT];
} pdirectory_t;

bool vmm_init();
bool vmm_alloc_page(pte_t* t_page);
bool vmm_free_page(pte_t* t_page);
pte_t* vmm_ptable_lookup(ptable_t *t_ptable, uint32_t t_addr);
pde_t* vmm_pdirectory_lookup(pdirectory_t *t_pdir, uint32_t t_addr);
pdirectory_t* vmm_get_current_pdir();
bool vmm_switch_pdir(pdirectory_t *t_pdir);
void vmm_enable_paging(bool enable);
bool vmm_map_page(uint32_t t_phyz, uint32_t t_virt);

bool vmm_load_page(uint32_t t_vert);
void vmm_page_fault_handler(uint8_t t_info, uint32_t t_virt);

#endif // __oneOS__MEM__VMM__VMM_H
