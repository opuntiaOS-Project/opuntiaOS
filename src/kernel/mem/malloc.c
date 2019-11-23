#include <mem/malloc.h>
#include <drivers/display.h>

uint32_t _kmalloc_start_position = 0;
bool sjiufjhwieufj = 0;

void kmalloc_init(uint32_t t_start_position) {
    _kmalloc_start_position = t_start_position;
}

void* kmalloc(uint32_t t_size) {
    uint32_t cur_ptr = _kmalloc_start_position;
    uint32_t alloc_size = 0;
    uint32_t reminder = (VMM_PAGE_SIZE - cur_ptr % VMM_PAGE_SIZE) % VMM_PAGE_SIZE;
    alloc_size += reminder;
    cur_ptr += reminder;

    while (t_size > alloc_size) {
        uint8_t* new_block = pmm_alloc_block();
        vmm_map_page(new_block, cur_ptr);
        cur_ptr += VMM_PAGE_SIZE;
        alloc_size += VMM_PAGE_SIZE;
    }
    uint32_t old_v = _kmalloc_start_position;
    _kmalloc_start_position += t_size;
    return old_v;
}

void kfree(void* t_size) {
    
}
