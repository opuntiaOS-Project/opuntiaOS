#include <mem/malloc.h>

uint32_t _kmalloc_t_start_position = 0;

void kmalloc_init(uint32_t t_start_position) {
    _kmalloc_t_start_position = t_start_position;
}

void* kmalloc(uint32_t t_size) {
    if (_kmalloc_t_start_position % 0x1000) {
        uint8_t* new_block = pmm_alloc_block();
        vmm_map_page(new_block, _kmalloc_t_start_position);
    }
    uint32_t old_v = _kmalloc_t_start_position;
    _kmalloc_t_start_position += t_size;
    return old_v;
}
