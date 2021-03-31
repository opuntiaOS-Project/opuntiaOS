
#include <mem/pmm.h>

mem_desc_t arm_mem_desc = {
    .memory_map_size = 2,
    .kernel_size = 500, // TODO: do it automatically
};

static memory_map_t arm_memmap_local[] = {
    {
        .startLo = 0x0,
        .startHi = 0x0,
        .sizeLo = 0x80000000, // 2GB
        .sizeHi = 0x0,
        .type = 0x0,
        .acpi_3_0 = 0x0,
    },
    {
        .startLo = 0x80000000, // 2GB
        .startHi = 0x0,
        .sizeLo = 0x7FFFFFFF, // 2GB
        .sizeHi = 0x0,
        .type = 0x1, // Free
        .acpi_3_0 = 0x0,
    },
};

struct memory_map* arm_memmap = (struct memory_map*)&arm_memmap_local;