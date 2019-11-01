#ifndef __oneOS__MEM__VMM__PDE_H
#define __oneOS__MEM__VMM__PDE_H

#include <types.h>

#define pde_t uint32_t

enum PDE_PAGE_FLAGS {
    PDE_PRESENT = 0,
    PDE_WRITABLE,
    PDE_USER,
    PDE_PWT,
    PDE_PCD,
    PDE_ACCESSED,
    PDE_DIRTY,
    PDE_4MB,
    PDE_CPU_GLOBAL,
    PDE_LV4_GLOBAL,
    PDE_FRAME = 12
};

void pde_set_attr(pde_t* t_pde, uint32_t t_attrs);
void pde_del_attr(pde_t* t_pde, uint32_t t_attrs);
void pde_set_frame(pde_t* t_pde, uint32_t frame);
void pde_del_frame(pde_t* t_pde);

bool pde_is_present(pde_t t_pde);
bool pde_is_writable(pde_t t_pde);
bool pde_is_4mb(pde_t t_pde);
uint32_t pde_get_frame(pde_t t_pde);

#endif //__oneOS__MEM__VMM__PDE_H
