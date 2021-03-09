#ifndef _LIBC_BITS_SYS_MMAN_H
#define _LIBC_BITS_SYS_MMAN_H

#include <sys/types.h>

#define MAP_SHARED 0x01
#define MAP_PRIVATE 0x02
#define MAP_FIXED 0x10
#define MAP_ANONYMOUS 0x20
#define MAP_STACK 0x40

#define PROT_READ 0x1
#define PROT_WRITE 0x2
#define PROT_EXEC 0x4
#define PROT_NONE 0x0

struct mmap_params {
    uint32_t addr;
    uint32_t size;
    uint32_t alignment;
    uint32_t prot;
    uint32_t flags;
    uint32_t fd;
    uint32_t offset;
};
typedef struct mmap_params mmap_params_t;

#endif // _LIBC_BITS_SYS_MMAN_H