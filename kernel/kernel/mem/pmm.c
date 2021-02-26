#include <libkern/log.h>
#include <mem/pmm.h>

// [Privates Prototypes]
static inline uint32_t _pmm_round_ceil(uint32_t value);
static inline uint32_t _pmm_round_floor(uint32_t value);
static inline void _pmm_mat_alloc_block(uint32_t block_id);
static inline void _pmm_mat_free_block(uint32_t block_id);
static inline bool _pmm_mat_tesblock(uint32_t block_id);
uint32_t _pmm_first_free_block();
uint32_t _pmm_first_free_block_n(uint32_t t_size);
void _pmm_init_region(uint32_t t_region_start, uint32_t t_region_length);
void _pmm_deinit_region(uint32_t t_region_start, uint32_t t_region_length);
void _pmm_deinit_mat();
void _pmm_calc_ram_size(mem_desc_t* mem_desc);
void _pmm_allocate_mat(void* t_mat_base);
static int last_i = 0;

static inline uint32_t _pmm_round_ceil(uint32_t value)
{
    if ((value & (PMM_BLOCK_SIZE - 1)) != 0) {
        value += PMM_BLOCK_SIZE;
        value &= (0xffffffff - (PMM_BLOCK_SIZE - 1));
    }
    return value;
}

static inline uint32_t _pmm_round_floor(uint32_t value)
{
    return (value & (0xffffffff - (PMM_BLOCK_SIZE - 1)));
}

// _pmm_mat_alloc_block sets in the MAT that the block is taken
static inline void _pmm_mat_alloc_block(uint32_t block_id)
{
    pmm_mat[block_id / PMM_BLOCKS_PER_BYTE] |= (1 << (block_id % PMM_BLOCKS_PER_BYTE));
}

// _pmm_mat_free_block sets in the MAT that the block is freed
static inline void _pmm_mat_free_block(uint32_t block_id)
{
    pmm_mat[block_id / PMM_BLOCKS_PER_BYTE] &= ~(1 << (block_id % PMM_BLOCKS_PER_BYTE));
}

// _pmm_mat_tesblock returns if the block is free
static inline bool _pmm_mat_tesblock(uint32_t block_id)
{
    return (pmm_mat[block_id / PMM_BLOCKS_PER_BYTE] >> (block_id % PMM_BLOCKS_PER_BYTE)) & 1;
}

// _pmm_first_free_block returns block_id
uint32_t _pmm_first_free_block()
{
    uint32_t* mat_big = (uint32_t*)pmm_mat;
    for (uint32_t i = last_i; i < pmm_mat_size / 4; i++) {
        // log("W %x", &mat_big[i]);
        if (mat_big[i] != 0xffffffff) {
            for (uint8_t j = 0; j < 32; j++) {
                uint32_t currenblock = i * 32 + j;
                if (!_pmm_mat_tesblock(currenblock)) {
                    // last_i = pmm_mat_size / 8;
                    return currenblock;
                }
            }
        }
    }
    return 0x0;
}

// _pmm_first_free_block_n returns block_id of block sequence
uint32_t _pmm_first_free_block_n(uint32_t t_size)
{
    if (t_size == 1) {
        return _pmm_first_free_block();
    }
    uint32_t* mat_big = (uint32_t*)pmm_mat;
    for (uint32_t i = last_i; i < pmm_mat_size / 4; i++) {
        if (mat_big[i] != 0xffffffff) {
            for (uint8_t j = 0; j < 32; j++) {
                uint32_t currenblock = i * 32 + j;
                uint32_t free = 0;
                for (uint32_t x = 0; x < t_size; x++) {
                    if (!_pmm_mat_tesblock(currenblock + x)) {
                        free++;
                    }
                }
                if (free == t_size) {
                    // last_i = pmm_mat_size / 8;
                    return currenblock;
                }
            }
        }
    }
    return 0x0;
}

uint32_t _pmm_first_free_block_n_aligned(uint32_t t_size, uint32_t alignment)
{
    uint32_t* mat_big = (uint32_t*)pmm_mat;
    for (uint32_t i = last_i; i < pmm_mat_size / 4; i++) {
        if (mat_big[i] != 0xffffffff) {
            for (uint8_t j = 0; j < 32; j++) {
                uint32_t currenblock = i * 32 + j;
                uint32_t free = 0;
                if (currenblock % alignment == 0) {
                    for (uint32_t x = 0; x < t_size; x++) {
                        if (!_pmm_mat_tesblock(currenblock + x)) {
                            free++;
                        }
                    }
                    if (free == t_size) {
                        // last_i = pmm_mat_size / 8;
                        return currenblock;
                    }
                }
            }
        }
    }
    return 0x0;
}

// _pmm_init_region marks the region as writable
void _pmm_init_region(uint32_t t_region_start, uint32_t t_region_length)
{
    t_region_start = _pmm_round_ceil(t_region_start);
    t_region_length = _pmm_round_floor(t_region_length);
    uint32_t block_id = t_region_start / PMM_BLOCK_SIZE;
    uint32_t blocks_count = t_region_length / PMM_BLOCK_SIZE;
    pmm_used_blocks -= blocks_count;
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

// _pmm_deinit_region marks the region as NOT writable
void _pmm_deinit_region(uint32_t t_region_start, uint32_t t_region_length)
{
    t_region_start = _pmm_round_floor(t_region_start);
    t_region_length = _pmm_round_ceil(t_region_length);
    uint32_t block_id = t_region_start / PMM_BLOCK_SIZE;
    uint32_t blocks_count = t_region_length / PMM_BLOCK_SIZE;
    pmm_used_blocks += blocks_count;
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

// _pmm_deinit_mat marks the region where MAT is placed as NOT writable
void _pmm_deinit_mat()
{
    _pmm_deinit_region((uint32_t)pmm_mat, pmm_mat_size);
}

// _pmm_calc_ram_size calculates ram size depends on the memory map
void _pmm_calc_ram_size(mem_desc_t* mem_desc)
{
    pmm_ram_size = 0;
    memory_map_t* memory_map = (memory_map_t*)MEMORY_MAP_REGION;
    for (int i = 0; i < mem_desc->memory_map_size; i++) {
        if (memory_map[i].type == 1) {
            pmm_ram_size = memory_map[i].startLo + memory_map[i].sizeLo;
        }
    }
}

// _pmm_allocate_mat puts MAT (Memory allocation table) in the ram
void _pmm_allocate_mat(void* t_mat_base)
{
    pmm_mat = t_mat_base;
    pmm_mat_size = pmm_ram_size / PMM_BLOCK_SIZE / PMM_BLOCKS_PER_BYTE;
    pmm_max_blocks = pmm_ram_size / PMM_BLOCK_SIZE;
    pmm_used_blocks = pmm_max_blocks;
    // mark all block as unavailable
    for (uint32_t i = 0; i < pmm_mat_size; ++i) {
        pmm_mat[i] = 0xff;
    }
}

void pmm_setup(mem_desc_t* mem_desc)
{
    uint32_t kernel_base_c = _pmm_round_ceil(KERNEL_BASE);
    uint32_t kernel_size = _pmm_round_ceil(mem_desc->kernel_size * 1024);
    _pmm_calc_ram_size(mem_desc);
    _pmm_allocate_mat((void*)(kernel_base_c + kernel_size));

    memory_map_t* memory_map = (memory_map_t*)MEMORY_MAP_REGION;
    for (int i = 0; i < mem_desc->memory_map_size; i++) {
        if (memory_map[i].type == 1) {
            log("  %d: %x - %x\n", i, memory_map[i].startLo, memory_map[i].sizeLo);
            _pmm_init_region(memory_map[i].startLo, memory_map[i].sizeLo);
        }
    }

    log("PMM: MAT size: %x", pmm_mat_size);

    // FIXME
#ifdef __i386__
    _pmm_deinit_region(0x0, 0x200000);
#elif __arm__
    _pmm_deinit_region(0x0, 0x80200000);
#endif
    // FIXME: DEINIT MAT DOES NOT WORK;
    _pmm_deinit_mat(); // mat deinit
    _pmm_deinit_region(0x0, KERNEL_PM_BASE); // kernel stack deinit
    _pmm_deinit_region(KERNEL_PM_BASE, mem_desc->kernel_size * 1024); // kernel deinit
}

// pmm_alloc_blocks allocates blocks
// will return 0x0 if unsuccesfully
void* pmm_alloc_blocks(uint32_t t_size)
{
    uint32_t block_id = _pmm_first_free_block_n(t_size);
    if (block_id == 0) {
        return 0x0;
    }
    for (uint32_t i = 0; i < t_size; i++) {
        _pmm_mat_alloc_block(block_id + i);
        pmm_used_blocks++;
    }
    return (void*)(block_id * PMM_BLOCK_SIZE);
}

void* pmm_alloc_blocks_aligned(uint32_t t_size, uint32_t al)
{
    uint32_t block_id = _pmm_first_free_block_n_aligned(t_size, al);
    if (block_id == 0) {
        return 0x0;
    }
    for (uint32_t i = 0; i < t_size; i++) {
        _pmm_mat_alloc_block(block_id + i);
        pmm_used_blocks++;
    }
    return (void*)(block_id * PMM_BLOCK_SIZE);
}

// pmm_free_blocks frees the blocks
// will return true if succesfully
// will return false if unsuccesfully
bool pmm_free_blocks(void* block, uint32_t t_size)
{
    if (((uint32_t)block & (PMM_BLOCK_SIZE - 1)) != 0) {
        return false;
    }
    uint32_t block_id = (uint32_t)block / PMM_BLOCK_SIZE;
    for (uint32_t i = 0; i < t_size; i++) {
        _pmm_mat_free_block(block_id + i);
        pmm_used_blocks--;
    }
}

// pmm_alloc_block allocates a block
// will return 0x0 if unsuccesfully
void* pmm_alloc_block()
{
    uint32_t block_id = _pmm_first_free_block();
    if (block_id == 0) {
        return 0x0;
    }
    _pmm_mat_alloc_block(block_id);
    pmm_used_blocks++;
    return (void*)(block_id * PMM_BLOCK_SIZE);
}

// pmm_alloc allocates space of @size bytes
void* pmm_alloc(uint32_t act_size)
{
    // log("MAT LOC %x", pmm_mat);
    uint32_t n = (act_size + PMM_BLOCK_SIZE - 1) / PMM_BLOCK_SIZE;
    return pmm_alloc_blocks(n);
}

void* pmm_alloc_aligned(uint32_t act_size, uint32_t alignment)
{
    // log("MAT LOC %x", pmm_mat);
    uint32_t n = (act_size + PMM_BLOCK_SIZE - 1) / PMM_BLOCK_SIZE;
    uint32_t al = (alignment + PMM_BLOCK_SIZE - 1) / PMM_BLOCK_SIZE;
    return pmm_alloc_blocks_aligned(n, al);
}

bool pmm_free(void* block, uint32_t act_size)
{
    uint32_t n = (act_size + PMM_BLOCK_SIZE - 1) / PMM_BLOCK_SIZE;
    return pmm_free_blocks(block, n);
}

// pmm_free_block frees the block
// will return true if succesfully
// will return false if unsuccesfully
bool pmm_free_block(void* block)
{
    if (((uint32_t)block & (PMM_BLOCK_SIZE - 1)) != 0) {
        return false;
    }
    uint32_t block_id = (uint32_t)block / PMM_BLOCK_SIZE;
    _pmm_mat_free_block(block_id);
    pmm_used_blocks--;
    return true;
}

uint32_t pmm_get_ram_size()
{
    return pmm_ram_size;
}

uint32_t pmm_get_max_blocks()
{
    return pmm_max_blocks;
}

uint32_t pmm_get_used_blocks()
{
    return pmm_used_blocks;
}

uint32_t pmm_get_free_blocks()
{
    return pmm_max_blocks - pmm_used_blocks;
}

uint32_t pmm_geblock_size()
{
    return PMM_BLOCK_SIZE;
}