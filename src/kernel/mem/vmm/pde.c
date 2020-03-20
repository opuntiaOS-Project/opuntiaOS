#include <mem/vmm/pde.h>

void table_desc_set_attr(table_desc_t* t_pde, uint32_t t_attrs) {
    *t_pde |= (1 << t_attrs);
}

void table_desc_del_attr(table_desc_t* t_pde, uint32_t t_attrs) {
    *t_pde &= ~(1 << t_attrs);
}

void table_desc_set_frame(table_desc_t* t_pde, uint32_t frame) {
    table_desc_del_frame(t_pde);
    *t_pde |= (frame << TABLE_DESC_FRAME);
}

void table_desc_del_frame(table_desc_t* t_pde) {
    *t_pde &= ((1 << (TABLE_DESC_FRAME)) - 1);
}

bool table_desc_is_present(table_desc_t t_pde) {
    return (t_pde >> TABLE_DESC_PRESENT) & 1;
}

bool table_desc_is_writable(table_desc_t t_pde) {
    return (t_pde >> TABLE_DESC_WRITABLE) & 1;
}

bool table_desc_is_4mb(table_desc_t t_pde) {
    return (t_pde >> TABLE_DESC_4MB) & 1;
}

bool table_desc_is_copy_on_write(table_desc_t pde) {
    return (pde >> TABLE_DESC_COPY_ON_WRITE) & 1;
}

bool table_desc_has_attr(table_desc_t pde, uint32_t attr) {
    return (pde >> attr) & 1;
}

uint32_t table_desc_get_frame(table_desc_t t_pde) {
    return (t_pde >> TABLE_DESC_FRAME) << 12;
}
