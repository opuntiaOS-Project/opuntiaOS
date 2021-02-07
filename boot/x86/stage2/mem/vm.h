#ifndef __oneLO__MEM__VMM__VMM_H
#define __oneLO__MEM__VMM__VMM_H

#include "../types.h"
#include "pte.h"
#include "pde.h"

#define VMM_PTE_COUNT (1024)
#define VMM_PDE_COUNT (1024)
#define VMM_PAGE_SIZE (4096)

enum VMM_ERR_CODES {
    VMM_ERR_PDIR,
    VMM_ERR_NO_SPACE,
    VMM_ERR_BAD_ADDR,
};

typedef struct {
    page_desc_t entities[VMM_PTE_COUNT];
} ptable_t;

typedef struct pdirectory {
    table_desc_t entities[VMM_PDE_COUNT];
} pdirectory_t;


int vm_setup();

#endif // __oneLO__MEM__VMM__VMM_H
