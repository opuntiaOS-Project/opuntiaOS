#ifndef _KERNEL_MEM_PMM_H
#define _KERNEL_MEM_PMM_H

#include <algo/bitmap.h>
#include <libkern/types.h>
#include <mem/boot.h>
#include <platform/generic/pmm/settings.h>

struct pmm_state {
    size_t kernel_va_base;
    size_t kernel_data_size; // Kernel + MAT size.

    bitmap_t mat;
    boot_desc_t* boot_desc;

    size_t ram_size;
    size_t ram_offset;

    size_t max_blocks;
    size_t used_blocks;
};
typedef struct pmm_state pmm_state_t;

void pmm_setup(boot_desc_t* boot_desc);

void* pmm_alloc(size_t size);
void* pmm_alloc_aligned(size_t size, size_t alignment);
int pmm_free(void* ptr, size_t size);

size_t pmm_get_ram_size();
size_t pmm_get_max_blocks();
size_t pmm_get_used_blocks();
size_t pmm_get_free_blocks();
size_t pmm_get_block_size();
size_t pmm_get_ram_in_kb();
size_t pmm_get_free_space_in_kb();
const pmm_state_t* pmm_get_state();

#endif // _KERNEL_MEM_PMM_H
