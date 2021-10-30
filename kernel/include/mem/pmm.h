#ifndef _KERNEL_MEM_PMM_H
#define _KERNEL_MEM_PMM_H

#include <libkern/types.h>
#include <platform/generic/pmm/settings.h>

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

static uint32_t pmm_ram_size;
static uint32_t pmm_max_blocks;
static uint32_t pmm_used_blocks;
static uint8_t* pmm_mat;
static uint32_t pmm_mat_size;

void pmm_setup(mem_desc_t* mem_desc);

void* pmm_alloc(uint32_t act_size);
void* pmm_alloc_aligned(uint32_t act_size, uint32_t alignment);
void* pmm_alloc_block();
void* pmm_alloc_blocks(uint32_t t_size);
bool pmm_free(void* block, uint32_t act_size);
bool pmm_free_block(void* t_block);
bool pmm_free_blocks(void* t_block, uint32_t t_size);

uint32_t pmm_get_ram_size();
uint32_t pmm_get_max_blocks();
uint32_t pmm_get_used_blocks();
uint32_t pmm_get_free_blocks();
uint32_t pmm_get_block_size();

uint32_t pmm_get_ram_in_kb();
uint32_t pmm_get_free_space_in_kb();

#endif // _KERNEL_MEM_PMM_H
