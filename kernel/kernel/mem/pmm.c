/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <algo/bitmap.h>
#include <libkern/libkern.h>
#include <libkern/log.h>
#include <mem/pmm.h>

// define DEBUG_PMM

static void _pmm_init_ram();
static void _pmm_allocate_mat();

static pmm_state_t pmm_state;

static inline void* _pmm_block_id_to_ptr(size_t value)
{
    return (void*)((value * PMM_BLOCK_SIZE) + pmm_state.ram_offset);
}

static inline size_t _pmm_ptr_to_block_id(void* value)
{
    return ((uintptr_t)value - pmm_state.ram_offset) / PMM_BLOCK_SIZE;
}

void _pmm_mark_avail_region(size_t region_start, size_t region_len)
{
    region_start = ROUND_CEIL(region_start, PMM_BLOCK_SIZE) - pmm_state.ram_offset;
    region_len = ROUND_FLOOR(region_len, PMM_BLOCK_SIZE);
#ifdef DEBUG_PMM
    log("PMM: Marked as avail: %x - %x", region_start + pmm_state.ram_offset, region_len);
#endif

    size_t block_id = region_start / PMM_BLOCK_SIZE;
    size_t blocks_len = region_len / PMM_BLOCK_SIZE;

    pmm_state.used_blocks -= blocks_len;
    bitmap_unset_range(pmm_state.mat, block_id, blocks_len);
}

void _pmm_mark_used_region(size_t region_start, size_t region_len)
{
    region_start = ROUND_FLOOR(region_start, PMM_BLOCK_SIZE) - pmm_state.ram_offset;
    region_len = ROUND_CEIL(region_len, PMM_BLOCK_SIZE);
#ifdef DEBUG_PMM
    log("PMM: Marked as used: %x - %x", region_start + pmm_state.ram_offset, region_len);
#endif

    size_t block_id = region_start / PMM_BLOCK_SIZE;
    size_t blocks_len = region_len / PMM_BLOCK_SIZE;

    pmm_state.used_blocks -= blocks_len;

    bitmap_set_range(pmm_state.mat, block_id, blocks_len);
}

static void _pmm_init_ram()
{
    pmm_state.ram_offset = (size_t)-1;
    pmm_state.ram_size = 0;

    memory_map_t* memory_map = (memory_map_t*)pmm_state.boot_desc->memory_map;
    for (int i = 0; i < pmm_state.boot_desc->memory_map_size; i++) {
        if (memory_map[i].type == 1) {
            pmm_state.ram_offset = min(pmm_state.ram_offset, memory_map[i].startLo);
            pmm_state.ram_size = max(pmm_state.ram_size, memory_map[i].startLo + memory_map[i].sizeLo);
        }
    }
}

static void _pmm_allocate_mat()
{
    size_t mat_cover_len = pmm_state.ram_size - pmm_state.ram_offset;
    pmm_state.mat = bitmap_wrap((void*)(pmm_state.kernel_va_base + pmm_state.kernel_size), mat_cover_len / PMM_BLOCK_SIZE);
    pmm_state.max_blocks = mat_cover_len / PMM_BLOCK_SIZE;
    pmm_state.used_blocks = pmm_state.max_blocks;
    memset(pmm_state.mat.data, 0xff, pmm_state.mat.len / 8);
}

static void _pmm_init_mat()
{
    memory_map_t* memory_map = (memory_map_t*)pmm_state.boot_desc->memory_map;
    for (int i = 0; i < pmm_state.boot_desc->memory_map_size; i++) {
        if (memory_map[i].type == 1) {
#ifdef DEBUG_PMM
            log("  %d: %x - %x", i, memory_map[i].startLo, memory_map[i].sizeLo);
#endif
            _pmm_mark_avail_region(memory_map[i].startLo, memory_map[i].sizeLo);
        }
    }
}

static void _pmm_init_from_desc(boot_desc_t* boot_desc)
{
    pmm_state.boot_desc = boot_desc;
    pmm_state.kernel_va_base = ROUND_CEIL(boot_desc->vaddr, PMM_BLOCK_SIZE);
    pmm_state.kernel_size = ROUND_CEIL(boot_desc->kernel_size * 1024, PMM_BLOCK_SIZE);

    _pmm_init_ram();
    _pmm_allocate_mat();
    _pmm_init_mat();

    // Marking all locations before kernel as used.
    if (boot_desc->paddr > pmm_state.ram_offset) {
        _pmm_mark_used_region(pmm_state.ram_offset, boot_desc->paddr - pmm_state.ram_offset);
    }

    // Marking kernel as used.
    _pmm_mark_used_region(boot_desc->paddr, boot_desc->kernel_size * 1024);

    // Marking MAT as used.
    size_t mat_pa_base = (size_t)pmm_state.mat.data - boot_desc->vaddr + boot_desc->paddr;
    _pmm_mark_used_region(mat_pa_base, pmm_state.mat.len / 8);
}

void pmm_setup(boot_desc_t* boot_desc)
{
    _pmm_init_from_desc(boot_desc);
}

static void* pmm_alloc_blocks(size_t count)
{
    int block_id = bitmap_find_space(pmm_state.mat, count);
    if (block_id < 0) {
        return NULL;
    }
    bitmap_set_range(pmm_state.mat, block_id, count);
    pmm_state.used_blocks += count;
    return _pmm_block_id_to_ptr(block_id);
}

static void* pmm_alloc_blocks_aligned(size_t count, size_t align)
{
    int block_id = bitmap_find_space_aligned(pmm_state.mat, count, align);
    if (block_id < 0) {
        return NULL;
    }

    bitmap_set_range(pmm_state.mat, block_id, count);
    pmm_state.used_blocks += count;
    return _pmm_block_id_to_ptr(block_id);
}

static int pmm_free_blocks(size_t block_id, size_t count)
{
    pmm_state.used_blocks -= count;
    return bitmap_unset_range(pmm_state.mat, block_id, count);
}

void* pmm_alloc(size_t size)
{
    size_t block_count = (size + PMM_BLOCK_SIZE - 1) / PMM_BLOCK_SIZE;
    return pmm_alloc_blocks(block_count);
}

void* pmm_alloc_aligned(size_t size, size_t align)
{
    size_t block_count = (size + PMM_BLOCK_SIZE - 1) / PMM_BLOCK_SIZE;
    size_t block_align = (align + PMM_BLOCK_SIZE - 1) / PMM_BLOCK_SIZE;
    if (block_align == 1) {
        return pmm_alloc_blocks(block_count);
    } else {
        return pmm_alloc_blocks_aligned(block_count, block_align);
    }
}

int pmm_free(void* block, size_t size)
{
    size_t block_id = _pmm_ptr_to_block_id(block);
    size_t block_count = (size + PMM_BLOCK_SIZE - 1) / PMM_BLOCK_SIZE;
    return pmm_free_blocks(block_id, block_count);
}

size_t pmm_get_ram_size()
{
    return pmm_state.ram_size;
}

size_t pmm_get_max_blocks()
{
    return pmm_state.max_blocks;
}

size_t pmm_get_used_blocks()
{
    return pmm_state.used_blocks;
}

size_t pmm_get_free_blocks()
{
    return pmm_get_max_blocks() - pmm_get_used_blocks();
}

size_t pmm_get_block_size()
{
    return PMM_BLOCK_SIZE;
}

size_t pmm_get_ram_in_kb()
{
    return pmm_get_ram_size() / 1024;
}

size_t pmm_get_free_space_in_kb()
{
    return pmm_get_free_blocks() * (PMM_BLOCK_SIZE / 1024);
}

const pmm_state_t* pmm_get_state()
{
    return &pmm_state;
}