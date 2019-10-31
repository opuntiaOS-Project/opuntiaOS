#ifndef __oneOS__MEM__PMM_H
#define __oneOS__MEM__PMM_H

#include <types.h>
#include <drivers/display.h>

#define PMM_BLOCK_SIZE (4096)
#define PMM_BLOCK_SIZE_KB (4)
#define PMM_BLOCKS_PER_BYTE (8)

static uint32_t pmm_ram_size;
static uint32_t pmm_max_blocks;
static uint32_t pmm_used_blocks;
static uint8_t* pmm_mat;
static uint32_t pmm_mat_size;

void pmm_init(uint32_t t_kernel_start, uint16_t t_kernel_size, uint32_t t_ram_size);
void pmm_init_region(uint32_t t_region_start, uint32_t t_region_length);
void pmm_deinit_region(uint32_t t_region_start, uint32_t t_region_length);

void* pmm_alloc_block();
void* pmm_alloc_blocks(uint32_t t_size);
bool pmm_free_block(void* t_block);
bool pmm_free_blocks(void* t_block, uint32_t t_size);


uint32_t pmm_get_ram_size();
uint32_t pmm_get_max_blocks();
uint32_t pmm_get_used_blocks();
uint32_t pmm_get_free_blocks();
uint32_t pmm_get_block_size();

void pmm_deinit_mat();

#endif // __oneOS__MEM__PMM_H
