#include <mem/vmm/pte.h>

void page_desc_set_attr(page_desc_t* t_pte, uint32_t t_attrs) {
    *t_pte |= (1 << t_attrs);
}

void page_desc_del_attr(page_desc_t* t_pte, uint32_t t_attrs) {
    *t_pte &= ~(1 << t_attrs);
}

void page_desc_set_frame(page_desc_t* t_pte, uint32_t frame) {
    page_desc_del_frame(t_pte);
    *t_pte |= (frame << PAGE_DESC_FRAME);
}

void page_desc_del_frame(page_desc_t* t_pte) {
    *t_pte &= ((1 << (PAGE_DESC_FRAME)) - 1);
}

bool page_desc_is_present(page_desc_t t_pte) {
    return t_pte & PAGE_DESC_PRESENT;
}

bool page_desc_is_writable(page_desc_t t_pte) {
    return ((t_pte & PAGE_DESC_WRITABLE) >> 1);
}

bool page_desc_is_user(page_desc_t t_pte) {
    return ((t_pte >> PAGE_DESC_USER) & 1);
}

bool page_desc_has_attr(page_desc_t pte, uint32_t attr) {
    return (pte >> attr) & 1;
}

uint32_t page_desc_get_frame(page_desc_t t_pte) {
    return (t_pte >> PAGE_DESC_FRAME) << 12;
}
