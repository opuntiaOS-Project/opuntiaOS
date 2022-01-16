#ifndef _KERNEL_MEM_BOOT_H
#define _KERNEL_MEM_BOOT_H

#include <libkern/types.h>

struct memory_map {
    uint32_t startLo;
    uint32_t startHi;
    uint32_t sizeLo;
    uint32_t sizeHi;
    uint32_t type;
    uint32_t acpi_3_0;
};
typedef struct memory_map memory_map_t;

struct boot_desc {
    size_t paddr;
    size_t vaddr;
    void* memory_map;
    size_t memory_map_size;
    size_t kernel_size;
    void* devtree;
};
typedef struct boot_desc boot_desc_t;

#endif // _KERNEL_MEM_BOOT_H