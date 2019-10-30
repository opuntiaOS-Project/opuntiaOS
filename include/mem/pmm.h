#ifndef __oneOS__MEM__PMM_H
#define __oneOS__MEM__PMM_H

#include <types.h>
#include <drivers/display.h>

#define PMM_BLOCK_SIZE (4096)
#define PMM_BLOCK_SIZE_KB (4)
#define PMM_BLOCKS_PER_BYTE (8)

static uint8_t* pmm_mat;
static uint32_t pmm_mat_size;

uint32_t _pmm_round_floor(uint32_t t_value);
uint32_t _pmm_round_ceil(uint32_t t_value);
void _pmm_mat_alloc_block(uint32_t t_block_id);
void _pmm_mat_free_block(uint32_t t_block_id);
bool _pmm_mat_test_block(uint32_t t_block_id);

void pmm_init(uint32_t t_kernel_start, uint16_t t_kernel_size, uint32_t t_ram_size);
void pmm_init_region(uint32_t t_region_start, uint32_t t_region_length);
void pmm_deinit_region(uint32_t t_region_start, uint32_t t_region_length);
void* pmm_alloc_block();
bool pmm_free_block(void* t_block);
void pmm_deinit_mat();

#endif // __oneOS__MEM__PMM_H
