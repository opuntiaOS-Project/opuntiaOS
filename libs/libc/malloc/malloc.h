#ifndef _LIBC_MALLOC_MALLOC_H
#define _LIBC_MALLOC_MALLOC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

#define MALLOC_DEFAULT_BLOCK_SIZE 4096
#define MALLOC_MAX_ALLOCATED_BLOCKS 64

#define FLAG_ALLOCATED (0x1)
#define FLAG_SLAB (0x2)
struct __malloc_header {
    size_t size;
    uint32_t flags;
    struct __malloc_header* next;
    struct __malloc_header* prev;
};
typedef struct __malloc_header malloc_header_t;

static inline bool block_has_flags(malloc_header_t* block, uint32_t flags)
{
    return ((block->flags & flags) == flags);
}

static inline void block_set_flags(malloc_header_t* block, uint32_t flags)
{
    block->flags |= flags;
}

static inline void block_rem_flags(malloc_header_t* block, uint32_t flags)
{
    block->flags &= (~flags);
}

static inline bool block_is_allocated(malloc_header_t* block)
{
    return block_has_flags(block, FLAG_ALLOCATED);
}

static inline bool block_is_free(malloc_header_t* block)
{
    return !block_has_flags(block, FLAG_ALLOCATED);
}

static inline bool block_is_slab(malloc_header_t* block)
{
    return block_has_flags(block, FLAG_SLAB);
}

void _malloc_init();
void _slab_init();

void* malloc(size_t);
void free(void*);
void* calloc(size_t, size_t);
void* realloc(void*, size_t);

void* slab_alloc(size_t);
void slab_free(malloc_header_t* mem_header);

__END_DECLS

#endif // _LIBC_MALLOC_MALLOC_H
