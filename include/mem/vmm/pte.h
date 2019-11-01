#ifndef __oneOS__MEM__VMM__PTE_H
#define __oneOS__MEM__VMM__PTE_H

#include <types.h>

#define pte_t uint32_t

enum PTE_PAGE_FLAGS {
    PTE_PRESENT = 0,
    PTE_WRITABLE,
    PTE_USER,
    PTE_WRITETHOUGH,
    PTE_NOT_CACHEABLE,
    PTE_ACCESSED,
    PTE_DIRTY,
    PTE_PAT,
    PTE_CPU_GLOBAL,
    PTE_LV4_GLOBAL,
    PTE_FRAME = 12
};

void pte_set_attr(pte_t* t_pte, uint32_t t_attrs);
void pte_del_attr(pte_t* t_pte, uint32_t t_attrs);
void pte_set_frame(pte_t* t_pte, uint32_t frame);
void pte_del_frame(pte_t* t_pte);

bool pte_is_present(pte_t t_pte);
bool pte_is_writable(pte_t t_pte);
uint32_t pte_get_frame(pte_t t_pte);

#endif //__oneOS__MEM__VMM__PTE_H
