#include <malloc.h>
#include <string.h>
#include <syscalls.h>

static void* memory[MALLOC_MAX_ALLOCATED_BLOCKS];
static uint8_t allocated_blocks;

static int __alloc_new_block(size_t sz);

static int __alloc_new_block(size_t sz)
{
    // this will reduce system calls for the small memory allocations
    size_t allocated_sz = sz > MALLOC_DEFAULT_BLOCK_SIZE ? sz : MALLOC_DEFAULT_BLOCK_SIZE;

    allocated_sz += sizeof(malloc_header_t);

    static mmap_params_t params;
    params.flags = MAP_ANONYMOUS | MAP_PRIVATE;
    params.size = allocated_sz;
    params.prot = PROT_READ | PROT_WRITE;

    int ret = mmap(&params);
    if (ret < 0) {
        return -1;
    }
    memory[allocated_blocks] = (void*)ret; // allocating a new block of memory
    ((malloc_header_t*)memory[allocated_blocks])->free = true;
    ((malloc_header_t*)memory[allocated_blocks])->next = 0;
    ((malloc_header_t*)memory[allocated_blocks])->size = allocated_sz - sizeof(malloc_header_t);

    allocated_blocks++;
    return 0;
}

void* malloc(size_t sz)
{
    /* iterating over allocated by mmap blocks
       and finding a first fit memory chunk
    */
    malloc_header_t* first_fit = 0;
    for (size_t i = 0; i < allocated_blocks; i++) {
        malloc_header_t* cur_block = memory[i];
        while (cur_block->next && cur_block->size < sz + sizeof(malloc_header_t)) {
            cur_block = cur_block->next;
        }
        if (cur_block->size + sizeof(malloc_header_t) >= sz) {
            first_fit = cur_block;
            break;
        }
    }

    if (!first_fit) {
        int err = __alloc_new_block(sz);
        if (err) {
            /* TODO: Write to log this */
            return 0;
        }
        first_fit = memory[allocated_blocks - 1];
    }

    malloc_header_t* copy_next = first_fit->next;
    size_t copy_size = first_fit->size;

    // setup a new chunk, separeted from the firstfit chunk
    first_fit->free = false;
    first_fit->size = sz;
    first_fit->next = (malloc_header_t*)((uint32_t)first_fit + sz + sizeof(malloc_header_t));

    // adjust the firstfit chunk
    first_fit->next->free = true;
    first_fit->next->size = copy_size - sz - sizeof(malloc_header_t);
    first_fit->next->next = copy_next;
    first_fit->next->prev = first_fit;

    // adjust a chunk placed next to the firstfit, if it exists
    if (first_fit->next->next) {
        first_fit->next->next->prev = first_fit->next;
    }

    return (void*)((uint32_t)first_fit + sizeof(malloc_header_t));
}

void free(void* mem)
{
    if (!mem) {
        return;
    }
    
    malloc_header_t* mem_header = (malloc_header_t*)((uint32_t)mem - sizeof(malloc_header_t));

    mem_header->free = true;

    // trying to glue freed chunk with its neigbors
    if (mem_header->next && mem_header->next->free) {
        mem_header->next = mem_header->next->next;
        mem_header->size += mem_header->next->size + sizeof(malloc_header_t);
    } else if (mem_header->prev && mem_header->prev->free) {
        mem_header->prev->size += mem_header->size + sizeof(malloc_header_t);
        mem_header->prev->next = mem_header->next;
    }
}

void* calloc(size_t num, size_t size)
{
    void* mem = malloc(num * size);
    memset(mem, 0, num * size);

    return mem;
}

void* realloc(void* ptr, size_t new_size)
{
    uint32_t old_size = ((malloc_header_t*)ptr)[-1].size;
    if (old_size == new_size) {
        return ptr;
    }

    uint8_t* new_area = malloc(new_size);
    if (!new_area) {
        return 0;
    }

    memcpy(new_area, ptr, new_size > old_size ? new_size : old_size);
    free(ptr);

    return new_area;
}