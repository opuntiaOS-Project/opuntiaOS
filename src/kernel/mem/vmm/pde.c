#include <mem/vmm/pde.h>

void pde_set_attr(pde_t* t_pde, uint32_t t_attrs) {
    *t_pde |= (1 << t_attrs);
}

void pde_del_attr(pde_t* t_pde, uint32_t t_attrs) {
    *t_pde &= ~(1 << t_attrs);
}

void pde_set_frame(pde_t* t_pde, uint32_t frame) {
    pde_del_frame(t_pde);
    *t_pde |= (frame << PDE_FRAME);
}

void pde_del_frame(pde_t* t_pde) {
    *t_pde &= ((1 << (PDE_FRAME)) - 1);
}

bool pde_is_present(pde_t t_pde) {
    return (t_pde >> PDE_PRESENT) & 1;
}

bool pde_is_writable(pde_t t_pde) {
    return (t_pde >> PDE_WRITABLE) & 1;
}

bool pde_is_4mb(pde_t t_pde) {
    return (t_pde >> PDE_4MB) & 1;
}

uint32_t pde_get_frame(pde_t t_pde) {
    return (t_pde >> PDE_FRAME);
}
