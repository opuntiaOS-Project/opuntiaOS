#ifndef __oneOS__MEM__VMM__PAGE_DESC_H
#define __oneOS__MEM__VMM__PAGE_DESC_H

#include <types.h>

#define page_desc_t uint32_t
#define pte_t uint32_t

enum PAGE_DESC_PAGE_FLAGS {
    PAGE_DESC_PRESENT = 0,
    PAGE_DESC_WRITABLE,
    PAGE_DESC_USER,
    PAGE_DESC_WRITETHOUGH,
    PAGE_DESC_NOT_CACHEABLE,
    PAGE_DESC_ACCESSED,
    PAGE_DESC_DIRTY,
    PAGE_DESC_PAT,
    PAGE_DESC_CPU_GLOBAL,
    PAGE_DESC_LV4_GLOBAL,
    PAGE_DESC_COPY_ON_WRITE,
    PAGE_DESC_FRAME = 12
};

void page_desc_set_attr(page_desc_t* t_pte, uint32_t t_attrs);
void page_desc_del_attr(page_desc_t* t_pte, uint32_t t_attrs);
void page_desc_set_frame(page_desc_t* t_pte, uint32_t frame);
void page_desc_del_frame(page_desc_t* t_pte);

bool page_desc_is_present(page_desc_t t_pte);
bool page_desc_is_writable(page_desc_t t_pte);
bool page_desc_is_user(page_desc_t t_pte);
uint32_t page_desc_get_frame(page_desc_t t_pte);

#endif //__oneOS__MEM__VMM__PAGE_DESC_H
