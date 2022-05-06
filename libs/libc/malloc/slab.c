#include "malloc.h"
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>

// Fast allocator for sizes 16, 32, 48, 64 bytes

malloc_header_t* free_blocks[5];

void prepare_free_blocks(size_t size)
{
    int block_id = size >> 4;
    const size_t alloc_size = MALLOC_DEFAULT_BLOCK_SIZE;
    intptr_t ret = (intptr_t)mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
    if (ret < 0) {
        free_blocks[block_id] = NULL;
        return;
    }

    void* raw_area = (void*)ret;
    const size_t sizeof_block_with_header = size + sizeof(malloc_header_t);
    size_t passed = 0;

    free_blocks[block_id] = (malloc_header_t*)raw_area;
    malloc_header_t* prev_header = NULL;

    while (passed + sizeof_block_with_header <= alloc_size) {
        malloc_header_t* current = (malloc_header_t*)raw_area;

        current->flags = FLAG_SLAB | FLAG_ALLOCATED;
        current->next = NULL;
        current->prev = prev_header;
        current->size = size;

        prev_header = current;
        raw_area += sizeof_block_with_header;
        passed += sizeof_block_with_header;
    }

    malloc_header_t* next_header = NULL;
    while (prev_header) {
        prev_header->next = next_header;
        prev_header = prev_header->prev;
    }
}

void _slab_init()
{
    prepare_free_blocks(16);
    prepare_free_blocks(32);
    prepare_free_blocks(48);
    prepare_free_blocks(64);
}

void* slab_alloc(size_t size)
{
    if (size > 64) {
        return NULL;
    }
    int block_id = ((size + 15) >> 4);
    malloc_header_t* zone = free_blocks[block_id];
    if (!zone) {
        return NULL;
    }

    free_blocks[block_id] = zone->next;
    return (void*)&((malloc_header_t*)zone)[1];
}

void slab_free(malloc_header_t* mem_header)
{
    malloc_header_t* next = free_blocks[((mem_header->size) >> 4)];
    free_blocks[((mem_header->size) >> 4)] = mem_header;
    mem_header->next = next;
    mem_header->prev = NULL;
    if (next) {
        next->prev = mem_header;
    }
}