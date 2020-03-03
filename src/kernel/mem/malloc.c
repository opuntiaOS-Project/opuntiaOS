#include <mem/malloc.h>
#include <drivers/display.h>

uint32_t _kmalloc_start_position = 0;

void kmalloc_init(uint32_t t_start_position) {
    _kmalloc_start_position = t_start_position;
}

void* kmalloc(uint32_t t_size) {
    uint32_t cur_ptr = _kmalloc_start_position;
    uint32_t old_size = t_size;
    uint32_t reminder = (VMM_PAGE_SIZE - cur_ptr % VMM_PAGE_SIZE) % VMM_PAGE_SIZE;
    uint32_t nxt_page = cur_ptr + reminder;

    while (t_size > reminder) {
        uint8_t* new_block = pmm_alloc_block();
        if (!vmm_map_page(new_block, nxt_page, 0, false)) {
            return 0;
        }
        nxt_page += VMM_PAGE_SIZE;
        t_size -= reminder;
        reminder = VMM_PAGE_SIZE;
    }

    _kmalloc_start_position += old_size;
    return cur_ptr;
}

void kfree(void* t_size) {
    
}
