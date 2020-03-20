#ifndef __oneOS__MEM__VMM__TABLE_DESC_H
#define __oneOS__MEM__VMM__TABLE_DESC_H

#include <types.h>

#define table_desc_t uint32_t
#define pde_t uint32_t

enum TABLE_DESC_PAGE_FLAGS {
    TABLE_DESC_PRESENT = 0,
    TABLE_DESC_WRITABLE,
    TABLE_DESC_USER,
    TABLE_DESC_PWT,
    TABLE_DESC_PCD,
    TABLE_DESC_ACCESSED,
    TABLE_DESC_DIRTY,
    TABLE_DESC_4MB,
    TABLE_DESC_CPU_GLOBAL,
    TABLE_DESC_LV4_GLOBAL,
    TABLE_DESC_COPY_ON_WRITE,
    TABLE_DESC_ZEROING_ON_DEMAND,
    TABLE_DESC_FRAME = 12
};

void table_desc_set_attr(table_desc_t* t_pde, uint32_t t_attrs);
void table_desc_del_attr(table_desc_t* t_pde, uint32_t t_attrs);
void table_desc_set_frame(table_desc_t* t_pde, uint32_t frame);
void table_desc_del_frame(table_desc_t* t_pde);

bool table_desc_has_attr(table_desc_t pde, uint32_t attr);
bool table_desc_is_present(table_desc_t t_pde);
bool table_desc_is_writable(table_desc_t t_pde);
bool table_desc_is_4mb(table_desc_t t_pde);
bool table_desc_is_copy_on_write(table_desc_t t_pde);
uint32_t table_desc_get_frame(table_desc_t t_pde);

#endif //__oneOS__MEM__VMM__TABLE_DESC_H
