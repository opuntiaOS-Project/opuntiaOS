#ifndef __oneOS__LibC__MALLOC_H
#define __oneOS__LibC__MALLOC_H

#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

#define MALLOC_DEFAULT_BLOCK_SIZE 4096
#define MALLOC_HEADER_SIZE 64
#define MALLOC_MAX_ALLOCATED_BLOCKS 64

struct __malloc_header {
    size_t size;
    int free; /* Should be bool, but now it's int for an alignment */
    struct __malloc_header* next;
    struct __malloc_header* prev;
};
typedef struct __malloc_header malloc_header_t;

void* malloc(size_t);
void free(void*);
void* calloc(size_t, size_t);
void* realloc(void*, size_t);

__END_DECLS

#endif // __oneOS__LibC__MALLOC_H
