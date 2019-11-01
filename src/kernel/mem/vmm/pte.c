#include <mem/vmm/pte.h>

void pte_set_attr(pte_t* t_pte, uint32_t t_attrs) {
    *t_pte |= (1 << t_attrs);
}

void pte_del_attr(pte_t* t_pte, uint32_t t_attrs) {
    *t_pte &= ~(1 << t_attrs);
}

void pte_set_frame(pte_t* t_pte, uint32_t frame) {
    pte_del_frame(t_pte);
    *t_pte |= (frame << PTE_FRAME);
}

void pte_del_frame(pte_t* t_pte) {
    *t_pte &= ((1 << (PTE_FRAME)) - 1);
}

bool pte_is_present(pte_t t_pte) {
    return t_pte & PTE_PRESENT;
}

bool pte_is_writable(pte_t t_pte) {
    return ((t_pte & PTE_WRITABLE) >> 1);
}

uint32_t pte_get_frame(pte_t t_pte) {
    return (t_pte >> PTE_FRAME);
}
