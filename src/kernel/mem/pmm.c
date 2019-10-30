#include <mem/pmm.h>

// [Private]
// _pmm_round_floor to round floor to the 4kb block bound
uint32_t _pmm_round_floor(uint32_t t_value) {
    return (t_value & (0xffffffff - 0xfff));
}

// [Private]
// _pmm_round_ceil to round ceil to the 4kb block bound
uint32_t _pmm_round_ceil(uint32_t t_value) {
    if ((t_value & 0xfff) != 0) {
        t_value += 0x1000;
        t_value &= (0xffffffff - 0xfff);
    }
    return t_value;
}

// [Private]
// _pmm_mat_alloc_block sets in the MAT that the block is taken
void _pmm_mat_alloc_block(uint32_t t_block_id) {
    pmm_mat[t_block_id / PMM_BLOCKS_PER_BYTE] |= (1 << (t_block_id % PMM_BLOCKS_PER_BYTE));
}

// [Private]
// _pmm_mat_alloc_block sets in the MAT that the block is freed
void _pmm_mat_free_block(uint32_t t_block_id) {
    pmm_mat[t_block_id / PMM_BLOCKS_PER_BYTE] &= ~(1 << (t_block_id % PMM_BLOCKS_PER_BYTE));
}

// [Private]
// _pmm_mat_test_block returns if the block is free
bool _pmm_mat_test_block(uint32_t t_block_id) {
    return (pmm_mat[t_block_id / PMM_BLOCKS_PER_BYTE] >> (t_block_id % PMM_BLOCKS_PER_BYTE)) & 1;
}

// pmm_init puts MAT (Memory allocation table) in the ram
void pmm_init(uint32_t t_kernel_start, uint16_t t_kernel_size_kb, uint32_t t_ram_size) {
    t_kernel_start = _pmm_round_floor(t_kernel_start);
    uint32_t kernel_size = t_kernel_size_kb * 1024;
    kernel_size = _pmm_round_ceil(kernel_size);
    pmm_mat = (void*)(t_kernel_start + kernel_size);
    pmm_mat_size = t_ram_size / PMM_BLOCK_SIZE / PMM_BLOCKS_PER_BYTE;
    // mark all block as unavailable
    for (uint32_t i = 0; i < pmm_mat_size; ++i) {
        pmm_mat[i] = 0xff;
    }
}

// pmm_init_region marks the region as writable
void pmm_init_region(uint32_t t_region_start, uint32_t t_region_length) {
    t_region_start = _pmm_round_ceil(t_region_start);
    t_region_length = _pmm_round_floor(t_region_length);
    uint32_t block_id = t_region_start / PMM_BLOCK_SIZE;
    uint32_t blocks_count = t_region_length / PMM_BLOCK_SIZE;
    while (blocks_count) {
        if (blocks_count >= PMM_BLOCKS_PER_BYTE && block_id % PMM_BLOCKS_PER_BYTE == 0) {
            pmm_mat[block_id / PMM_BLOCKS_PER_BYTE] = 0;
            blocks_count -= PMM_BLOCKS_PER_BYTE;
            block_id += PMM_BLOCKS_PER_BYTE;
        } else {
            _pmm_mat_free_block(block_id);
            blocks_count -= 1;
            block_id += 1;
        }
    }
}

// pmm_deinit_region marks the region as NOT writable
void pmm_deinit_region(uint32_t t_region_start, uint32_t t_region_length) {
    t_region_start = _pmm_round_floor(t_region_start);
    t_region_length = _pmm_round_ceil(t_region_length);
    uint32_t block_id = t_region_start / PMM_BLOCK_SIZE;
    uint32_t blocks_count = t_region_length / PMM_BLOCK_SIZE;
    while (blocks_count) {
        if (blocks_count >= PMM_BLOCKS_PER_BYTE && block_id % PMM_BLOCKS_PER_BYTE == 0) {
            pmm_mat[block_id / PMM_BLOCKS_PER_BYTE] = 0xff;
            blocks_count -= PMM_BLOCKS_PER_BYTE;
            block_id += PMM_BLOCKS_PER_BYTE;
        } else {
            _pmm_mat_alloc_block(block_id);
            blocks_count -= 1;
            block_id += 1;
        }
    }
}

// pmm_deinit_mat marks the region where MAT is placed as NOT writable
void pmm_deinit_mat() {
    pmm_deinit_region(pmm_mat, pmm_mat_size);
}

// pmm_alloc_block allocates a block
// will return 0xffffffff if unsuccesfully
void* pmm_alloc_block() {
    uint32_t *mat_big = pmm_mat;
    for (uint32_t i = 0; i < pmm_mat_size / 4; i++) {
        if (mat_big[i] != 0xffffffff) {
            for (uint8_t j = 0; j < 32; j++) {
                uint32_t current_block = i * 32 + j;
                if (!_pmm_mat_test_block(current_block)) {
                    _pmm_mat_alloc_block(current_block);
                    return PMM_BLOCK_SIZE * current_block;
                }
            }
        }
    }
    return 0xffffffff;
}

// pmm_free_block frees the block
// will return true if succesfully
// will return false if unsuccesfully
bool pmm_free_block(void* t_block) {
    if (((uint32_t)t_block & 0xfff) != 0) {
        return false;
    }
    _pmm_mat_free_block((uint32_t)t_block / PMM_BLOCK_SIZE);
    return true;
}
