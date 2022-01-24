#ifndef _KERNEL_MEM_PMM_H
#define _KERNEL_MEM_PMM_H

#include <libkern/types.h>
#include <mem/boot.h>
#include <platform/generic/pmm/settings.h>

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

#endif // _KERNEL_MEM_PMM_H
