#include "malloc.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>

#define ALIGNMENT (4)
#define DEVIDE_SPACE_BOUNDARY (32)

static malloc_header_t* memory[MALLOC_MAX_ALLOCATED_BLOCKS];
static size_t allocated_blocks = 0;

static int _alloc_new_block(size_t sz);

static int _alloc_new_block(size_t sz)
{
    sz += sizeof(malloc_header_t);

    // this will reduce system calls for the small memory allocations
    size_t allocated_sz = sz > MALLOC_DEFAULT_BLOCK_SIZE ? sz : MALLOC_DEFAULT_BLOCK_SIZE;

    int ret = (int)mmap(NULL, allocated_sz, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
    if (ret < 0) {
        return -1;
    }
    memory[allocated_blocks] = (void*)ret; // allocating a new block of memory
    memory[allocated_blocks]->flags = 0;
    memory[allocated_blocks]->next = 0;
    memory[allocated_blocks]->prev = 0;
    memory[allocated_blocks]->size = allocated_sz - sizeof(malloc_header_t);

    allocated_blocks++;
    return 0;
}

static inline char _malloc_need_to_divide_space(malloc_header_t* space, size_t alloc_size)
{
    return alloc_size + DEVIDE_SPACE_BOUNDARY <= space->size;
}

static inline char _malloc_can_fit_allocation(malloc_header_t* space, size_t alloc_size)
{
    uint32_t add[] = { 0, sizeof(malloc_header_t) };
    return space->size >= (alloc_size + add[_malloc_need_to_divide_space(space, alloc_size)]);
}

void* malloc(size_t sz)
{
    if (!sz) {
        return NULL;
    }
    sz += (ALIGNMENT - 1);
    sz &= ~(uint32_t)(ALIGNMENT - 1);

    void* res = slab_alloc(sz);
    if (res) {
        return res;
    }

    /* iterating over allocated by mmap blocks
       and finding a first fit memory chunk */
    malloc_header_t* first_fit = 0;
    for (size_t i = 0; i < allocated_blocks; i++) {
        malloc_header_t* cur_block = memory[i];
        while (cur_block->next && !(block_is_free(cur_block) && _malloc_can_fit_allocation(cur_block, sz))) {
            cur_block = cur_block->next;
        }
        if (block_is_free(cur_block) && _malloc_can_fit_allocation(cur_block, sz)) {
            first_fit = cur_block;
            break;
        }
    }

    if (!first_fit) {
        int err = _alloc_new_block(sz);
        if (err) {
            return NULL;
        }
        first_fit = memory[allocated_blocks - 1];
    }

    malloc_header_t* copy_next = first_fit->next;
    size_t copy_size = first_fit->size;

    first_fit->flags |= FLAG_ALLOCATED;
    if (_malloc_need_to_divide_space(first_fit, sz)) {
        first_fit->size = sz;
        first_fit->next = (malloc_header_t*)((uintptr_t)first_fit + sz + sizeof(malloc_header_t));

        // adjust the firstfit chunk
        first_fit->next->flags = 0;
        first_fit->next->size = copy_size - sz - sizeof(malloc_header_t);
        first_fit->next->next = copy_next;
        first_fit->next->prev = first_fit;

        if (first_fit->next->next) {
            first_fit->next->next->prev = first_fit->next;
        }
    }

    return (void*)&((malloc_header_t*)first_fit)[1];
}

void free(void* mem)
{
    if (!mem) {
        return;
    }

    malloc_header_t* mem_header = &((malloc_header_t*)mem)[-1];

    if (block_is_slab(mem_header)) {
        return slab_free(mem_header);
    }

    block_rem_flags(mem_header, FLAG_ALLOCATED);
    while (mem_header->prev && block_is_free(mem_header->prev)) {
        mem_header = mem_header->prev;
    }

    // Trying to glue the freed chunk with its neighbours.
    while (mem_header->next && block_is_free(mem_header->next)) {
        mem_header->size += mem_header->next->size + sizeof(malloc_header_t);

        if (mem_header->next->next) {
            mem_header->next->next->prev = mem_header;
        }
        mem_header->next = mem_header->next->next;
    }
}

void* calloc(size_t num, size_t size)
{
    void* mem = malloc(num * size);
    if (!mem) {
        return NULL;
    }

    memset(mem, 0, num * size);
    return mem;
}

void* realloc(void* ptr, size_t new_size)
{
    if (!ptr) {
        return malloc(new_size);
    }

    size_t old_size = ((malloc_header_t*)ptr)[-1].size;
    if (old_size == new_size) {
        return ptr;
    }

    uint8_t* new_area = malloc(new_size);
    if (!new_area) {
        return NULL;
    }

    memcpy(new_area, ptr, new_size < old_size ? new_size : old_size);
    free(ptr);

    return new_area;
}

void _malloc_init()
{
    _slab_init();
}