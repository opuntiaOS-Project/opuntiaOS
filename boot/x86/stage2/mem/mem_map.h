#ifndef _BOOT_X86_STAGE2_MEM_MEM_MAP_H
#define _BOOT_X86_STAGE2_MEM_MEM_MAP_H

#include "../types.h"

typedef struct {
    uint32_t startLo;
    uint32_t startHi;
    uint32_t sizeLo;
    uint32_t sizeHi;
    uint32_t type;
    uint32_t acpi_3_0;
} memory_map_t;

typedef struct {
    uint16_t memory_map_size;
    uint16_t kernel_size;
} mem_desc_t;

#endif // _BOOT_X86_STAGE2_MEM_MEM_MAP_H