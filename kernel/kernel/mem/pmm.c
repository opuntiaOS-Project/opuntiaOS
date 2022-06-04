/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <algo/bitmap.h>
#include <libkern/libkern.h>
#include <libkern/lock.h>
#include <libkern/log.h>
#include <mem/pmm.h>

#define DEBUG_PMM

static void _pmm_init_ram();
static void _pmm_allocate_mat();

static pmm_state_t pmm_state;
static spinlock_t _pmm_global_lock;

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
    log("PMM: Marked as avail: %zx - %zx", region_start + pmm_state.ram_offset, region_len);
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
    log("PMM: Marked as used: %zx - %zx", region_start + pmm_state.ram_offset, region_len);
#endif

    size_t block_id = region_start / PMM_BLOCK_SIZE;
    size_t blocks_len = region_len / PMM_BLOCK_SIZE;

    pmm_state.used_blocks -= blocks_len;

    bitmap_set_range(pmm_state.mat, block_id, blocks_len);
}

static void _pmm_init_ram()
{
    pmm_state.ram_offset = pmm_state.boot_args->mem_boot_desc.ram_base;
    pmm_state.ram_size = pmm_state.boot_args->mem_boot_desc.ram_size;

    //     pmm_state.boot_args->mem_boot_desc.reserved_areas
    //     memory_map_t* memory_map = (memory_map_t*)pmm_state.boot_args->mem;
    //     for (int i = 0; i < pmm_state.boot_args->memory_map_size; i++) {
    //         if (memory_map[i].type == 1) {
    //             uint64_t start = ((uint64_t)memory_map[i].startHi << 32) + memory_map[i].startLo;
    //             uint64_t size = ((uint64_t)memory_map[i].sizeHi << 32) + memory_map[i].sizeLo;
    //             pmm_state.ram_offset = min(pmm_state.ram_offset, start);

    // #ifdef DEBUG_PMM
    //             log("Pmm desc: %zx %zx", start, size);
    // #endif

    //             // This is a hack, since memory map differs a lot for x86 and arm.
    //             // Might need to unify this somehow (maybe pass info from bootloader?).
    // #ifdef __i386__
    //             pmm_state.ram_size = max(pmm_state.ram_size, start + memory_map[i].sizeLo);
    // #elif __arm__
    //             pmm_state.ram_size += memory_map[i].sizeLo;
    // #elif __aarch64__
    //             pmm_state.ram_size += size;
    // #endif
    //         }
    //     }
}

static void _pmm_allocate_mat()
{
    size_t mat_cover_len = pmm_state.ram_size;
    pmm_state.mat = bitmap_wrap((void*)(pmm_state.kernel_va_base + pmm_state.kernel_data_size), mat_cover_len / PMM_BLOCK_SIZE);
    pmm_state.max_blocks = mat_cover_len / PMM_BLOCK_SIZE;
    pmm_state.used_blocks = pmm_state.max_blocks;
#ifdef DEBUG_PMM
    log("Allocated MAT @ %p - %zx", pmm_state.mat.data, pmm_state.mat.len);
#endif
    memset(pmm_state.mat.data, 0xff, pmm_state.mat.len / 8);
    pmm_state.kernel_data_size += ROUND_CEIL(pmm_state.mat.len / 8, PMM_BLOCK_SIZE);
}

static void _pmm_init_mat()
{
    _pmm_mark_avail_region(pmm_state.ram_offset, pmm_state.ram_size);

    memory_layout_t* mem_layout = (memory_layout_t*)pmm_state.boot_args->mem_boot_desc.reserved_areas;
    for (int i = 0;; i++) {
        if (TEST_FLAG(mem_layout[i].flags, MEMORY_LAYOUT_FLAG_TERMINATE)) {
            break;
        }

        _pmm_mark_used_region(mem_layout[i].base, mem_layout[i].size);
    }

    //     memory_map_t* memory_map = (memory_map_t*)pmm_state.boot_args->memory_map;
    //     for (int i = 0; i < pmm_state.boot_args->memory_map_size; i++) {
    //         if (memory_map[i].type == 1) {
    //             uint64_t start = ((uint64_t)memory_map[i].startHi << 32) + memory_map[i].startLo;
    //             uint64_t size = ((uint64_t)memory_map[i].sizeHi << 32) + memory_map[i].sizeLo;
    // #ifdef DEBUG_PMM
    //             log("  %d: %zx - %zx", i, start, size);
    // #endif
    //             _pmm_mark_avail_region(start, size);
    //         }
    //     }
}

static void _pmm_init_from_desc(boot_args_t* boot_args)
{
    spinlock_init(&_pmm_global_lock);
    pmm_state.boot_args = boot_args;
    pmm_state.kernel_va_base = ROUND_FLOOR(boot_args->vaddr, PMM_BLOCK_SIZE);
    pmm_state.kernel_data_size = ROUND_CEIL(boot_args->kernel_data_size, PMM_BLOCK_SIZE);

    _pmm_init_ram();
    _pmm_allocate_mat();
    _pmm_init_mat();

    // Marking all locations before kernel as used.
    if (boot_args->paddr > pmm_state.ram_offset) {
        _pmm_mark_used_region(pmm_state.ram_offset, boot_args->paddr - pmm_state.ram_offset);
    }

    // Marking all kernel data as used.
    _pmm_mark_used_region(boot_args->paddr, pmm_state.kernel_data_size);
}

void pmm_setup(boot_args_t* boot_args)
{
    _pmm_init_from_desc(boot_args);
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

void* pmm_alloc_locked(size_t size)
{
    size_t block_count = (size + PMM_BLOCK_SIZE - 1) / PMM_BLOCK_SIZE;
    return pmm_alloc_blocks(block_count);
}

void* pmm_alloc_aligned_locked(size_t size, size_t align)
{
    size_t block_count = (size + PMM_BLOCK_SIZE - 1) / PMM_BLOCK_SIZE;
    size_t block_align = (align + PMM_BLOCK_SIZE - 1) / PMM_BLOCK_SIZE;
    if (block_align == 1) {
        return pmm_alloc_blocks(block_count);
    }
    return pmm_alloc_blocks_aligned(block_count, block_align);
}

int pmm_free_locked(void* block, size_t size)
{
    size_t block_id = _pmm_ptr_to_block_id(block);
    size_t block_count = (size + PMM_BLOCK_SIZE - 1) / PMM_BLOCK_SIZE;
    return pmm_free_blocks(block_id, block_count);
}

void* pmm_alloc(size_t size)
{
    spinlock_acquire(&_pmm_global_lock);
    void* res = pmm_alloc_locked(size);
    spinlock_release(&_pmm_global_lock);
    return res;
}

void* pmm_alloc_aligned(size_t size, size_t align)
{
    spinlock_acquire(&_pmm_global_lock);
    void* res = pmm_alloc_aligned_locked(size, align);
    spinlock_release(&_pmm_global_lock);
    return res;
}

int pmm_free(void* block, size_t size)
{
    spinlock_acquire(&_pmm_global_lock);
    int res = pmm_free_locked(block, size);
    spinlock_release(&_pmm_global_lock);
    return res;
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

const boot_args_t* boot_args()
{
    return pmm_state.boot_args;
}