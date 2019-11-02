#ifndef __oneOS__MEM__MALLOC_H
#define __oneOS__MEM__MALLOC_H

#include <types.h>
#include <mem/vmm/vmm.h>


void kmalloc_init(uint32_t t_start_position);
void* kmalloc(uint32_t t_size);

#endif // __oneOS__MEM__MALLOC_H
