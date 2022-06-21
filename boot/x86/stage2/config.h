#ifndef STAGE2_CONFIG
#define STAGE2_CONFIG

#include <libboot/types.h>

#define KERNEL_BASE 0x100000

struct x86_memory_map {
    uint32_t startLo;
    uint32_t startHi;
    uint32_t sizeLo;
    uint32_t sizeHi;
    uint32_t type;
    uint32_t acpi_3_0;
};
typedef struct x86_memory_map x86_memory_map_t;

struct x86_mem_desc {
    uint32_t memory_map_size;
    uint32_t kernel_size;
};
typedef struct x86_mem_desc x86_mem_desc_t;

#endif // STAGE2_CONFIG