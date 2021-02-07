/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <errno.h>
#include <log.h>
#include <mem/kmalloc.h>
#include <mem/vmm/vmm.h>
#include <mem/vmm/zoner.h>
#include <platform/generic/system.h>
#include <platform/generic/vmm/pf_types.h>
#include <tasking/tasking.h>
#include <utils.h>

// #define VMM_DEBUG

#define pdir_t pdirectory_t
#define VMM_TOTAL_PAGES_PER_TABLE VMM_PTE_COUNT
#define VMM_TOTAL_TABLES_PER_DIRECTORY VMM_PDE_COUNT
#define PDIR_SIZE sizeof(pdirectory_t)
#define PTABLE_SIZE sizeof(ptable_t)
#define IS_INDIVIDUAL_PER_DIR(index) (index < VMM_KERNEL_TABLES_START || (index == VMM_OFFSET_IN_DIRECTORY(pspace_zone.start)))

static pdir_t* _vmm_kernel_pdir;
static pdir_t* _vmm_active_pdir;
static zone_t pspace_zone;
static uint32_t kernel_ptables_start_paddr = 0x0;

#define vmm_kernel_pdir_phys2virt(paddr) ((void*)((uint32_t)paddr + KERNEL_BASE - KERNEL_PM_BASE))

/**
 * PRIVATE FUNCTIONS
 */

inline static void* _vmm_pspace_get_vaddr_of_active_pdir();
inline static void* _vmm_pspace_get_nth_active_ptable(uint32_t n);
inline static void* _vmm_pspace_get_vaddr_of_active_ptable(uint32_t vaddr);
static bool _vmm_split_pspace();
static void _vmm_pspace_init();
static void _vmm_pspace_gen(pdirectory_t*);
static void* _vmm_kernel_convert_vaddr2paddr(uint32_t vaddr);
static void* _vmm_convert_vaddr2paddr(uint32_t vaddr);

inline static void* _vmm_alloc_kernel_pdir();
inline static uint32_t _vmm_alloc_pdir_paddr();
inline static uint32_t _vmm_alloc_ptable_paddr();
inline static uint32_t _vmm_alloc_page_paddr();
static bool _vmm_init_switch_to_kernel_pdir();
static void _vmm_map_init_kernel_pages(uint32_t paddr, uint32_t vaddr);
static bool _vmm_create_kernel_ptables();
static bool _vmm_map_kernel();

inline static uint32_t _vmm_round_ceil_to_page(uint32_t value);
inline static uint32_t _vmm_round_floor_to_page(uint32_t value);

static bool _vmm_is_copy_on_write(uint32_t vaddr);
static int _vmm_resolve_copy_on_write(proc_t* p, uint32_t vaddr);
static void _vmm_ensure_cow_for_page(uint32_t vaddr);
static void _vmm_ensure_cow_for_range(uint32_t vaddr, uint32_t length);
static int _vmm_copy_page_to_resolve_cow(proc_t* p, uint32_t vaddr, ptable_t* src_ptable, int page_index);

static bool _vmm_is_zeroing_on_demand(uint32_t vaddr);
static void _vmm_resolve_zeroing_on_demand(uint32_t vaddr);

static int _vmm_self_test();

/**
 * VM INITIALIZATION FUNCTIONS
 */

inline static void* _vmm_alloc_kernel_pdir()
{
    uint32_t paddr = (uint32_t)pmm_alloc_aligned(PDIR_SIZE, PDIR_SIZE);
    return vmm_kernel_pdir_phys2virt(paddr);
}

inline static uint32_t _vmm_alloc_pdir_paddr()
{
    return (uint32_t)pmm_alloc_aligned(PDIR_SIZE, PDIR_SIZE);
}

inline static uint32_t _vmm_alloc_ptable_paddr()
{
    return (uint32_t)pmm_alloc(PTABLE_SIZE);
}

inline static uint32_t _vmm_alloc_ptables_to_cover_page()
{
    return (uint32_t)pmm_alloc_aligned(VMM_PAGE_SIZE, VMM_PAGE_SIZE);
}

inline static uint32_t _vmm_alloc_page_paddr()
{
    return (uint32_t)pmm_alloc_aligned(VMM_PAGE_SIZE, VMM_PAGE_SIZE);
}

static zone_t _vmm_alloc_mapped_zone(uint32_t size, uint32_t alignment)
{
    if (size % VMM_PAGE_SIZE) {
        size += VMM_PAGE_SIZE - (size % VMM_PAGE_SIZE);
    }
    if (alignment % VMM_PAGE_SIZE) {
        alignment += VMM_PAGE_SIZE - (alignment % VMM_PAGE_SIZE);
    }

    // TODO: Currently only sequence allocation is implemented.
    zone_t zone = zoner_new_zone_aligned(size, alignment);
    uint32_t paddr = (uint32_t)pmm_alloc_aligned(size, alignment);
    vmm_map_pages(zone.start, paddr, size / VMM_PAGE_SIZE, PAGE_READABLE | PAGE_WRITABLE);
    return zone;
}

static int _vmm_free_mapped_zone(zone_t zone)
{
    ptable_t* ptable = (ptable_t*)_vmm_pspace_get_vaddr_of_active_ptable(zone.start);
    page_desc_t* page = vmm_ptable_lookup(ptable, zone.start);
    pmm_free((void*)page_desc_get_frame(*page), zone.len);
    vmm_unmap_pages(zone.start, zone.len / VMM_PAGE_SIZE);
    zoner_free_zone(zone);
    return 0;
}

inline static pdirectory_t* _vmm_alloc_pdir()
{
    zone_t zone = _vmm_alloc_mapped_zone(PDIR_SIZE, PDIR_SIZE);
    return (pdirectory_t*)zone.ptr;
}

inline static void _vmm_free_pdir(pdirectory_t* pdir)
{
    if (pdir == _vmm_kernel_pdir) {
        return;
    }

    zone_t zone;
    zone.start = (uint32_t)pdir;
    zone.len = PDIR_SIZE;
    _vmm_free_mapped_zone(zone);
}

/**
 * PSPACE FUNCTIONS
 * 
 * Pspace is a space with all current tables mapped there.
 * The space starts from @pspace_zone.start and length is 4mb
 */

/**
 * The function is supposed to setup pspace_zone.start
 * Every virtual space has its own user's ptables in the area.
 * The kernel's patables are the same.
 */

inline static void* _vmm_pspace_get_vaddr_of_active_pdir()
{
    return (void*)_vmm_active_pdir;
}

inline static void* _vmm_pspace_get_nth_active_ptable(uint32_t n)
{
    return (void*)(pspace_zone.start + n * PTABLE_SIZE);
}

inline static void* _vmm_pspace_get_vaddr_of_active_ptable(uint32_t vaddr)
{
    return (void*)_vmm_pspace_get_nth_active_ptable(VMM_OFFSET_IN_DIRECTORY(vaddr));
}

static bool _vmm_split_pspace()
{
    pspace_zone = zoner_new_zone(4 * MB);

    if (VMM_OFFSET_IN_TABLE(pspace_zone.start) != 0) {
        kpanic("WRONG PSPACE START ADDR");
    }

    _vmm_kernel_pdir = (pdir_t*)_vmm_alloc_kernel_pdir();
    _vmm_active_pdir = (pdir_t*)_vmm_kernel_pdir;
    memset((void*)_vmm_active_pdir, 0, PDIR_SIZE);
}

/**
 * The function is used to init pspace.
 * Used only in the first stage of VM init.
 */
static void _vmm_pspace_init()
{
    /* The code assumes that the length of tables which cover pspace
       is 4KB and that the tables are fit in a single page and are continuous. */
    uint32_t kernel_ptabels_vaddr = pspace_zone.start + VMM_KERNEL_TABLES_START * PTABLE_SIZE; // map what
    uint32_t kernel_ptabels_paddr = kernel_ptables_start_paddr; // map to
    uint32_t ptables_per_page = VMM_PAGE_SIZE / PTABLE_SIZE;
    for (int i = VMM_KERNEL_TABLES_START; i < VMM_TOTAL_TABLES_PER_DIRECTORY; i += ptables_per_page) {
        table_desc_t* ptable_desc = vmm_pdirectory_lookup(_vmm_active_pdir, kernel_ptabels_vaddr);
        if (!table_desc_is_present(*ptable_desc)) {
            // must present
            kpanic("PSPACE_6335 : BUG\n");
        }
        ptable_t* ptable_vaddr = (ptable_t*)(kernel_ptables_start_paddr + (VMM_OFFSET_IN_DIRECTORY(kernel_ptabels_vaddr) - VMM_KERNEL_TABLES_START) * PTABLE_SIZE);
        page_desc_t* page = vmm_ptable_lookup(ptable_vaddr, kernel_ptabels_vaddr);
        page_desc_set_attrs(page, PAGE_DESC_PRESENT | PAGE_DESC_WRITABLE);
        page_desc_set_frame(page, kernel_ptabels_paddr);
        kernel_ptabels_vaddr += VMM_PAGE_SIZE;
        kernel_ptabels_paddr += VMM_PAGE_SIZE;
    }
}

/**
 * The function is used to generate a new pspace.
 * The function returns the table of itself. 
 * Pspace table is one page (4KB) long, since the whole length is 4MB.
 */
static void _vmm_pspace_gen(pdirectory_t* pdir)
{
    ptable_t* cur_ptable = (ptable_t*)_vmm_pspace_get_nth_active_ptable(VMM_OFFSET_IN_DIRECTORY(pspace_zone.start));
    uint32_t ptables_per_page = VMM_PAGE_SIZE / PTABLE_SIZE;
    uint32_t ptable_paddr = _vmm_alloc_ptables_to_cover_page();
    zone_t tmp_zone = zoner_new_zone(VMM_PAGE_SIZE);
    ptable_t* new_ptable = (ptable_t*)tmp_zone.start;

    vmm_map_page((uint32_t)new_ptable, ptable_paddr, PAGE_READABLE | PAGE_WRITABLE);

    /* The code assumes that the length of tables which cover pspace
       is 4KB and that the tables are fit in a single page and are continuous. */
    memcpy(new_ptable, cur_ptable, VMM_PAGE_SIZE);

    page_desc_t pspace_page;
    page_desc_init(&pspace_page);
    page_desc_set_attrs(&pspace_page, PAGE_DESC_PRESENT | PAGE_DESC_WRITABLE);
    page_desc_set_frame(&pspace_page, ptable_paddr);

    /* According to prev comment, we can remain overflow here, since we write to the right memory cell */
    new_ptable->entities[VMM_OFFSET_IN_DIRECTORY(pspace_zone.start) / ptables_per_page] = pspace_page;

    uint32_t table_coverage = VMM_PAGE_SIZE * VMM_TOTAL_PAGES_PER_TABLE;
    uint32_t ptable_vaddr_for = pspace_zone.start;
    uint32_t ptable_paddr_for = ptable_paddr;
    for (int i = 0; i < ptables_per_page; i++, ptable_vaddr_for += table_coverage, ptable_paddr_for += PTABLE_SIZE) {
        table_desc_t pspace_table;
        table_desc_init(&pspace_table);
        table_desc_set_attrs(&pspace_table, TABLE_DESC_PRESENT | TABLE_DESC_WRITABLE);
        table_desc_set_frame(&pspace_table, ptable_paddr_for);
        pdir->entities[VMM_OFFSET_IN_DIRECTORY(ptable_vaddr_for)] = pspace_table;
    }
    vmm_unmap_page((uint32_t)new_ptable);
    zoner_free_zone(tmp_zone);
}

static void _vmm_free_pspace(pdirectory_t* pdir)
{
    table_desc_t* ptable_desc = &pdir->entities[VMM_OFFSET_IN_DIRECTORY(pspace_zone.start)];
    if (!table_desc_has_attrs(*ptable_desc, TABLE_DESC_PRESENT)) {
        return;
    }
    pmm_free_block((void*)table_desc_get_frame(*ptable_desc));
    table_desc_del_frame(ptable_desc);
}

/**
 * The function is used to traslate a virtual address into physical
 * Used only in the first stage of VM init
 */
static void* _vmm_kernel_convert_vaddr2paddr(uint32_t vaddr)
{
    ptable_t* ptable_paddr = (ptable_t*)(kernel_ptables_start_paddr + (VMM_OFFSET_IN_DIRECTORY(vaddr) - VMM_KERNEL_TABLES_START) * PTABLE_SIZE);
    page_desc_t* page_desc = vmm_ptable_lookup(ptable_paddr, vaddr);
    return (void*)((page_desc_get_frame(*page_desc)) | (vaddr & 0xfff));
}

/**
 * The function is used to traslate a virtual address into physical
 */
static void* _vmm_convert_vaddr2paddr(uint32_t vaddr)
{
    ptable_t* ptable_vaddr = (ptable_t*)_vmm_pspace_get_vaddr_of_active_ptable(vaddr);
    page_desc_t* page_desc = vmm_ptable_lookup(ptable_vaddr, vaddr);
    return (void*)((page_desc_get_frame(*page_desc)) | (vaddr & 0xfff));
}

/**
 * The function is used to update active pdir
 * Used only in the first stage of VM init
 */
static bool _vmm_init_switch_to_kernel_pdir()
{
    _vmm_active_pdir = _vmm_kernel_pdir;
    system_disable_interrupts();
    system_set_pdir((uint32_t)_vmm_kernel_convert_vaddr2paddr((uint32_t)_vmm_active_pdir));
    system_enable_interrupts();
    return true;
}

/**
 * The function is used to map kernel pages
 * Used only in the first stage of VM init
 */
static void _vmm_map_init_kernel_pages(uint32_t paddr, uint32_t vaddr)
{
    ptable_t* ptable_paddr = (ptable_t*)(kernel_ptables_start_paddr + (VMM_OFFSET_IN_DIRECTORY(vaddr) - VMM_KERNEL_TABLES_START) * PTABLE_SIZE);
    for (uint32_t phyz = paddr, virt = vaddr, i = 0; i < VMM_TOTAL_PAGES_PER_TABLE; phyz += VMM_PAGE_SIZE, virt += VMM_PAGE_SIZE, i++) {
        page_desc_t new_page;
        page_desc_init(&new_page);
        page_desc_set_attrs(&new_page, PAGE_DESC_PRESENT | PAGE_DESC_WRITABLE);
        page_desc_set_frame(&new_page, phyz);
        ptable_paddr->entities[i] = new_page;
    }
}

/**
 * The function is supposed to create all kernel tables and map necessary
 * data into _vmm_kernel_pdir.
 * Used only in the first stage of VM init
 */
static bool _vmm_create_kernel_ptables()
{
    uint32_t table_coverage = VMM_PAGE_SIZE * VMM_TOTAL_PAGES_PER_TABLE;
    uint32_t kernel_ptabels_vaddr = VMM_KERNEL_TABLES_START * table_coverage;

    for (int i = VMM_KERNEL_TABLES_START; i < VMM_TOTAL_TABLES_PER_DIRECTORY; i++, kernel_ptabels_vaddr += table_coverage) {
        // updating table descriptor of kernel's pdir
        table_desc_t* ptable_desc = vmm_pdirectory_lookup(_vmm_kernel_pdir, kernel_ptabels_vaddr);
        uint32_t paddr = _vmm_alloc_ptable_paddr();
        if (!paddr) {
            kpanic("PADDR_5546 : BUG\n");
        }

        if (!kernel_ptables_start_paddr) {
            kernel_ptables_start_paddr = paddr;
        }
        table_desc_init(ptable_desc);
        table_desc_set_attrs(ptable_desc, TABLE_DESC_PRESENT | TABLE_DESC_WRITABLE);

        /**
         * VMM_OFFSET_IN_DIRECTORY(pspace_zone.start) shows number of table where pspace starts. 
         * Since pspace is right after kernel, let's protect them and not give user access to the whole
         * ptable (and since ptable is not user, all pages inside it are not user too).
         */
        if (i > VMM_OFFSET_IN_DIRECTORY(pspace_zone.start)) {
            table_desc_set_attrs(ptable_desc, TABLE_DESC_USER);
        }

        table_desc_set_frame(ptable_desc, paddr);
    }

    // TODO: Replace with table
#ifdef __i386__
    _vmm_map_init_kernel_pages(0x00100000, 0xc0000000);
    _vmm_map_init_kernel_pages(0x00000000, 0xffc00000);
#elif __arm__
    _vmm_map_init_kernel_pages(0x80100000, 0xc0000000); // kernel 1mb
    _vmm_map_init_kernel_pages(0x80200000, 0xc0100000); // kernel
    _vmm_map_init_kernel_pages(0x80300000, 0xc0200000); // kernel
    _vmm_map_init_kernel_pages(0x80400000, 0xc0300000); // kernel
    _vmm_map_init_kernel_pages(0x80500000, 0xc0400000); // kernel
    _vmm_map_init_kernel_pages(0x80600000, 0xc0500000); // kernel
    _vmm_map_init_kernel_pages(0x80700000, 0xc0600000); // kernel
    _vmm_map_init_kernel_pages(0x80800000, 0xc0700000); // kernel
#endif

    return true;
}

/**
 * The function is supposed to map secondary data for kernel's pdir.
 * Used only in the first stage of VM init
 */
static bool _vmm_map_kernel()
{
#ifdef __i386__
    vmm_map_pages(0x00000000, 0x00000000, 1024, PAGE_READABLE | PAGE_WRITABLE | PAGE_EXECUTABLE);
#elif __arm__
    vmm_map_page(0x1c090000, 0x1c090000, PAGE_READABLE | PAGE_WRITABLE | PAGE_EXECUTABLE);
    vmm_map_page(0x3F00b000, 0x3F00b000, PAGE_READABLE | PAGE_WRITABLE | PAGE_EXECUTABLE);
#endif
    return true;
}

int vmm_setup()
{
    zoner_init(0xc0400000);
    _vmm_split_pspace();
    _vmm_create_kernel_ptables();
    _vmm_pspace_init();
    _vmm_init_switch_to_kernel_pdir();
    _vmm_map_kernel();
    zoner_place_bitmap();
    kmalloc_init();
    return 0;
}

/**
 * VM TOOLS
 */

inline static uint32_t _vmm_round_ceil_to_page(uint32_t value)
{
    if ((value & (VMM_PAGE_SIZE - 1)) != 0) {
        value += VMM_PAGE_SIZE;
        value &= (0xffffffff - (VMM_PAGE_SIZE - 1));
    }
    return value;
}

inline static uint32_t _vmm_round_floor_to_page(uint32_t value)
{
    return (value & (0xffffffff - (VMM_PAGE_SIZE - 1)));
}

table_desc_t* vmm_pdirectory_lookup(pdirectory_t* pdir, uint32_t vaddr)
{
    if (pdir) {
        return &pdir->entities[VMM_OFFSET_IN_DIRECTORY(vaddr)];
    }
    return 0;
}

page_desc_t* vmm_ptable_lookup(ptable_t* ptable, uint32_t vaddr)
{
    if (ptable) {
        return &ptable->entities[VMM_OFFSET_IN_TABLE(vaddr)];
    }
    return 0;
}

/**
 * The function is supposed to create new ptable (not kernel) and
 * rebuild pspace to match to the new setup.
 * It may allocate several tables to cover the full page. For example,
 * it allocates 4 tables for arm (ptable -- 1KB, while a page is 4KB).
 */
int vmm_allocate_ptable(uint32_t vaddr)
{
    if (!_vmm_active_pdir) {
        return -VMM_ERR_PDIR;
    }

    uint32_t ptables_paddr = _vmm_alloc_ptables_to_cover_page();
    if (!ptables_paddr) {
        log_error(" vmm_allocate_ptable: No free space in pmm to alloc ptables");
        return -VMM_ERR_NO_SPACE;
    }

    uint32_t ptable_vaddr_start = PAGE_START((uint32_t)_vmm_pspace_get_vaddr_of_active_ptable(vaddr));
    uint32_t ptables_per_page = VMM_PAGE_SIZE / PTABLE_SIZE;
    uint32_t table_coverage = VMM_PAGE_SIZE * VMM_TOTAL_PAGES_PER_TABLE;
    uint32_t ptable_serve_vaddr_start = (vaddr / (table_coverage * ptables_per_page)) * (table_coverage * ptables_per_page);

    for (uint32_t i = 0, pvaddr = ptable_serve_vaddr_start, ptable_paddr = ptables_paddr; i < ptables_per_page; i++, pvaddr += table_coverage, ptable_paddr += PTABLE_SIZE) {
        table_desc_t* ptable_desc = vmm_pdirectory_lookup(_vmm_active_pdir, pvaddr);
        table_desc_clear(ptable_desc);
        table_desc_init(ptable_desc);
        table_desc_set_attrs(ptable_desc, TABLE_DESC_PRESENT | TABLE_DESC_WRITABLE | TABLE_DESC_USER);
        table_desc_set_frame(ptable_desc, ptable_paddr);
    }

    int ret = vmm_map_page(ptable_vaddr_start, ptables_paddr, PAGE_READABLE | PAGE_WRITABLE | PAGE_EXECUTABLE | PAGE_CHOOSE_OWNER(vaddr));
    if (ret == 0) {
        memset((void*)ptable_vaddr_start, 0, VMM_PAGE_SIZE);
    }
    return ret;
}

static int vmm_free_ptable(uint32_t ptable_indx)
{
    if (_vmm_active_pdir == 0) {
        return -VMM_ERR_PDIR;
    }

    table_desc_t* ptable_desc = &_vmm_active_pdir->entities[ptable_indx];

    if (!table_desc_has_attrs(*ptable_desc, TABLE_DESC_PRESENT)) {
        return -EFAULT;
    }

    if (table_desc_has_attrs(*ptable_desc, TABLE_DESC_COPY_ON_WRITE)) {
        return -EFAULT;
    }

    pmm_free_block((void*)table_desc_get_frame(*ptable_desc));
    table_desc_del_frame(ptable_desc);

    return 0;
}

/**
 * The function is supposed to map vaddr to paddr
 */

int vmm_map_page(uint32_t vaddr, uint32_t paddr, uint32_t settings)
{
    if (!_vmm_active_pdir) {
        return -VMM_ERR_PDIR;
    }

    bool is_writable = ((settings & PAGE_WRITABLE) > 0);
    bool is_readable = ((settings & PAGE_READABLE) > 0);
    bool is_executable = ((settings & PAGE_EXECUTABLE) > 0);
    bool is_not_cacheable = ((settings & PAGE_NOT_CACHEABLE) > 0);
    bool is_cow = ((settings & PAGE_COW) > 0);
    bool is_user = ((settings & PAGE_USER) > 0);

    table_desc_t* ptable_desc = vmm_pdirectory_lookup(_vmm_active_pdir, vaddr);
    if (!table_desc_is_present(*ptable_desc)) {
        vmm_allocate_ptable(vaddr);
    }

    ptable_t* ptable = (ptable_t*)_vmm_pspace_get_vaddr_of_active_ptable(vaddr);
    page_desc_t* page = vmm_ptable_lookup(ptable, vaddr);

    page_desc_init(page);
    page_desc_set_attrs(page, PAGE_DESC_PRESENT);
    page_desc_set_frame(page, paddr);

    // If we are in COW, no write is allowed anyway.
    if (is_writable && !table_desc_is_copy_on_write(*ptable_desc)) {
        page_desc_set_attrs(page, PAGE_DESC_WRITABLE);
    }

    if (is_user) {
        page_desc_set_attrs(page, PAGE_DESC_USER);
    }

    if (is_not_cacheable) {
        page_desc_set_attrs(page, PAGE_DESC_NOT_CACHEABLE);
    }

#ifdef VMM_DEBUG
    log("Page mapped %x in pdir: %x", vaddr, vmm_get_active_pdir());
#endif

    system_flush_tlb_entry(vaddr);

    return 0;
}

int vmm_unmap_page(uint32_t vaddr)
{
    if (!_vmm_active_pdir) {
        return -VMM_ERR_PDIR;
    }

    table_desc_t* ptable_desc = vmm_pdirectory_lookup(_vmm_active_pdir, vaddr);
    if (!table_desc_is_present(*ptable_desc)) {
        return -VMM_ERR_PTABLE;
    }

    ptable_t* ptable = (ptable_t*)_vmm_pspace_get_vaddr_of_active_ptable(vaddr);
    page_desc_t* page = vmm_ptable_lookup(ptable, vaddr);
    page_desc_del_attrs(page, PAGE_DESC_PRESENT);
    page_desc_del_attrs(page, PAGE_DESC_WRITABLE);
    page_desc_del_frame(page);
    system_flush_tlb_entry(vaddr);

    return 0;
}

/**
 * The function is supposed to map a sequence of vaddrs to paddrs
 */
int vmm_map_pages(uint32_t vaddr, uint32_t paddr, uint32_t n_pages, uint32_t settings)
{
    if ((paddr & 0xfff) || (vaddr & 0xfff)) {
        return -VMM_ERR_BAD_ADDR;
    }

    int status = 0;
    for (; n_pages; paddr += VMM_PAGE_SIZE, vaddr += VMM_PAGE_SIZE, n_pages--) {
        if (status = vmm_map_page(vaddr, paddr, settings) < 0) {
            return status;
        }
    }

    return 0;
}

int vmm_unmap_pages(uint32_t vaddr, uint32_t n_pages)
{
    if (vaddr & 0xfff) {
        return -VMM_ERR_BAD_ADDR;
    }

    int status = 0;
    for (; n_pages; vaddr += VMM_PAGE_SIZE, n_pages--) {
        if (status = vmm_unmap_page(vaddr) < 0) {
            return status;
        }
    }

    return 0;
}

/**
 * COPY ON WRITE FUNCTIONS
 */

/**
 * Marks 2 tables as copy-on-write tables.
 */
static inline void _vmm_tables_set_cow(uint32_t table_index, table_desc_t* cur, table_desc_t* new)
{
#ifdef __i386__
    table_desc_del_attrs(cur, TABLE_DESC_WRITABLE);
    table_desc_set_attrs(cur, TABLE_DESC_COPY_ON_WRITE);
    table_desc_del_attrs(new, TABLE_DESC_WRITABLE);
    table_desc_set_attrs(new, TABLE_DESC_COPY_ON_WRITE);
#elif __arm__
    table_desc_set_attrs(cur, TABLE_DESC_COPY_ON_WRITE);
    table_desc_set_attrs(new, TABLE_DESC_COPY_ON_WRITE);

    /* Marking all pages as not-writable to handle COW. Later will restore using zones data */
    ptable_t* ptable = _vmm_pspace_get_nth_active_ptable(table_index);
    // log("Ask set cow %x", ptable);
    for (int i = 0; i < VMM_TOTAL_PAGES_PER_TABLE; i++) {
        page_desc_t* page = &ptable->entities[i];
        page_desc_del_attrs(page, PAGE_DESC_WRITABLE);
    }
#endif
}

static bool _vmm_is_copy_on_write(uint32_t vaddr)
{
    table_desc_t* ptable_desc = vmm_pdirectory_lookup(_vmm_active_pdir, vaddr);
    return table_desc_is_copy_on_write(*ptable_desc);
}

// TODO handle delete of tables and dirs
static int _vmm_resolve_copy_on_write(proc_t* p, uint32_t vaddr)
{
    uint32_t ptables_per_page = VMM_PAGE_SIZE / PTABLE_SIZE;
    uint32_t table_coverage = VMM_PAGE_SIZE * VMM_TOTAL_PAGES_PER_TABLE;
    uint32_t ptable_serve_vaddr_start = (vaddr / (table_coverage * ptables_per_page)) * (table_coverage * ptables_per_page);

    /* Copying old ptables which cover the full page. See a comment above vmm_allocate_ptable. */
    zone_t src_ptable_zone = _vmm_alloc_mapped_zone(VMM_PAGE_SIZE, VMM_PAGE_SIZE);
    ptable_t* src_ptable = (ptable_t*)src_ptable_zone.ptr;
    ptable_t* root_ptable = (ptable_t*)PAGE_START((uint32_t)_vmm_pspace_get_vaddr_of_active_ptable(ptable_serve_vaddr_start));
    memcpy(src_ptable, root_ptable, VMM_PAGE_SIZE);
    // log("Resolve got %x", root_ptable);

    /* Setting up new ptables. */
    int res = vmm_allocate_ptable(vaddr);
    if (res != 0) {
        return res;
    }

    ptable_t* new_ptable = _vmm_pspace_get_vaddr_of_active_ptable(vaddr);

    /* Copying all pages from this tables to newly allocated. */
    uint32_t table_start = TABLE_START(ptable_serve_vaddr_start);
    for (int i = 0; i < VMM_TOTAL_PAGES_PER_TABLE * ptables_per_page; i++) {
        uint32_t page_vaddr = table_start + (i * VMM_PAGE_SIZE);
        page_desc_t* page_desc = &src_ptable->entities[i];
        if (page_desc_is_present(*page_desc)) {
            // log("       PAGE %x", *page_desc);
            _vmm_copy_page_to_resolve_cow(p, page_vaddr, src_ptable, i);
        }
    }

    _vmm_free_mapped_zone(src_ptable_zone);
}

static void _vmm_ensure_cow_for_page(uint32_t vaddr)
{
    if (_vmm_is_copy_on_write(vaddr)) {
        proc_t* holder_proc = tasking_get_proc_by_pdir(vmm_get_active_pdir());
        if (!holder_proc) {
            kpanic("No proc with the pdir\n");
        }
        _vmm_resolve_copy_on_write(holder_proc, vaddr);
    }
}

static void _vmm_ensure_cow_for_range(uint32_t vaddr, uint32_t length)
{
    uint32_t page_addr = PAGE_START(vaddr);
    while (page_addr < vaddr + length) {
        _vmm_ensure_cow_for_page(page_addr);
        page_addr += VMM_PAGE_SIZE;
    }
}

/**
 * The function is supposed to copy a page from @src_ptable to active
 * ptable. The page which is copied has address @vaddr. ONLY TO RESOLVE COW!
 */
static int _vmm_copy_page_to_resolve_cow(proc_t* p, uint32_t vaddr, ptable_t* src_ptable, int page_index)
{
    page_desc_t* old_page_desc = &src_ptable->entities[page_index];

    proc_zone_t* zone = proc_find_zone(p, vaddr);
    if (!zone) {
        log_error("Cow: No page in zone");
        return SHOULD_CRASH;
    }
    // log("COW: RESTORE PAGE %x with %x", vaddr, zone->flags);
    vmm_load_page(vaddr, zone->flags);

    /* Mapping the old page to do a copy */
    zone_t tmp_zone = zoner_new_zone(VMM_PAGE_SIZE);
    uint32_t old_page_vaddr = (uint32_t)tmp_zone.start;
    uint32_t old_page_paddr = page_desc_get_frame(*old_page_desc);
    vmm_map_page(old_page_vaddr, old_page_paddr, PAGE_READABLE);

    /*  We don't need to check if there is a problem with cow, since this is a special copy function
        to resolve cow. So, we know that pages were created recently. Checking cow here would cause an
        ub, since table has not been setup correctly yet. */
    memcpy((uint8_t*)vaddr, (uint8_t*)old_page_vaddr, VMM_PAGE_SIZE);

    /* Freeing */
    vmm_unmap_page(old_page_vaddr);
    zoner_free_zone(tmp_zone);
    return 0;
}

/**
 * ZEROING ON DEMAND FUNCTIONS
 */

// static bool _vmm_is_zeroing_on_demand(uint32_t vaddr)
// {
//     table_desc_t* ptable_desc = vmm_pdirectory_lookup(_vmm_active_pdir, vaddr);
//     if (table_desc_has_attrs(*ptable_desc, TABLE_DESC_ZEROING_ON_DEMAND)) {
//         return true;
//     }
//     ptable_t* ptable = _vmm_pspace_get_vaddr_of_active_ptable(vaddr);
//     table_desc_t* ppage_desc = vmm_ptable_lookup(ptable, vaddr);
//     return page_desc_has_attrs(*ppage_desc, PAGE_DESC_ZEROING_ON_DEMAND);
// }

// static void _vmm_resolve_zeroing_on_demand(uint32_t vaddr)
// {
//     table_desc_t* ptable_desc = (table_desc_t*)vmm_pdirectory_lookup(_vmm_active_pdir, vaddr);
//     ptable_t* ptable = _vmm_pspace_get_vaddr_of_active_ptable(vaddr);

//     if (table_desc_has_attrs(*ptable_desc, TABLE_DESC_ZEROING_ON_DEMAND)) {
//         for (int i = 0; i < VMM_TOTAL_PAGES_PER_TABLE; i++) {
//             page_desc_set_attrs(&ptable->entities[i], PAGE_DESC_ZEROING_ON_DEMAND);
//             page_desc_del_attrs(&ptable->entities[i], PAGE_DESC_WRITABLE);
//         }
//         table_desc_del_attrs(ptable_desc, TABLE_DESC_ZEROING_ON_DEMAND);
//     }

//     table_desc_t* ppage_desc = (table_desc_t*)vmm_ptable_lookup(ptable, vaddr);

//     uint8_t* dest = (uint8_t*)_vmm_round_floor_to_page(vaddr);
//     memset(dest, 0, VMM_PAGE_SIZE);

//     page_desc_del_attrs(ppage_desc, PAGE_DESC_ZEROING_ON_DEMAND);
//     page_desc_set_attrs(ppage_desc, PAGE_DESC_WRITABLE);
// }

/**
 * USER PDIR FUNCTIONS
 */

/**
 * The function is supposed to create all new user's pdir.
 */
pdirectory_t* vmm_new_user_pdir()
{
    pdirectory_t* pdir = _vmm_alloc_pdir();

    for (int i = VMM_USER_TABLES_START; i < VMM_KERNEL_TABLES_START; i++) {
        table_desc_clear(&pdir->entities[i]);
    }

    for (int i = VMM_KERNEL_TABLES_START; i < VMM_TOTAL_TABLES_PER_DIRECTORY; i++) {
        if (!IS_INDIVIDUAL_PER_DIR(i)) {
            pdir->entities[i] = _vmm_kernel_pdir->entities[i];
        }
    }

    _vmm_pspace_gen(pdir);
    return pdir;
}

/**
 * The function created a new user's pdir from the active pdir.
 * After copying the task we need to flush tlb. To do that we have to
 * to reload cr3 tlb entries. 
 */
pdirectory_t* vmm_new_forked_user_pdir()
{
    pdirectory_t* new_pdir = _vmm_alloc_pdir();

    // coping all tables
    for (int i = 0; i < VMM_TOTAL_TABLES_PER_DIRECTORY; i++) {
        new_pdir->entities[i] = _vmm_active_pdir->entities[i];
    }
    _vmm_pspace_gen(new_pdir);

    for (int i = 0; i < VMM_KERNEL_TABLES_START; i++) {
        table_desc_t* act_ptable_desc = &_vmm_active_pdir->entities[i];
        if (table_desc_has_attrs(*act_ptable_desc, TABLE_DESC_PRESENT)) {
            table_desc_t* new_ptable_desc = &new_pdir->entities[i];
            _vmm_tables_set_cow(i, act_ptable_desc, new_ptable_desc);
        }
    }

    system_flush_whole_tlb();

    return new_pdir;
}

int vmm_free_pdir(pdirectory_t* pdir)
{
    if (!pdir) {
        return -EINVAL;
    }

    if (pdir == _vmm_kernel_pdir) {
        return -EACCES;
    }

    pdirectory_t* cur_pdir = vmm_get_active_pdir();
    vmm_switch_pdir(pdir);

    for (int i = 0; i < VMM_KERNEL_TABLES_START; i++) {
        vmm_free_ptable(i);
    }

    if (cur_pdir == pdir) {
        vmm_switch_pdir(_vmm_kernel_pdir);
    } else {
        vmm_switch_pdir(cur_pdir);
    }
    _vmm_free_pspace(pdir);
    _vmm_free_pdir(pdir);
    return 0;
}

void* vmm_bring_to_kernel(uint8_t* src, uint32_t length)
{
    if ((uint32_t)src >= KERNEL_BASE) {
        return src;
    }
    uint8_t* kaddr = kmalloc(length);
    memcpy(kaddr, src, length);
    return (void*)kaddr;
}

void vmm_fast_copy_to_active_pdir(void* src, uint32_t dest_vaddr, uint32_t length)
{
    _vmm_ensure_cow_for_range(dest_vaddr, length);
    uint8_t* dest = (uint8_t*)dest_vaddr;
    memcpy(dest, src, length);
}

void vmm_copy_to_user(void* dest, void* src, uint32_t length)
{
    _vmm_ensure_cow_for_range((uint32_t)dest, length);
    memcpy(dest, src, length);
}

void vmm_copy_to_pdir(pdirectory_t* pdir, uint8_t* src, uint32_t dest_vaddr, uint32_t length)
{
    pdirectory_t* prev_pdir = vmm_get_active_pdir();
    uint8_t* ksrc;
    if ((uint32_t)src < KERNEL_BASE) {
        /* Means that it's in user space, so it should be copied here. */
        ksrc = vmm_bring_to_kernel(src, length);
    } else {
        ksrc = src;
    }

    vmm_switch_pdir(pdir);

    _vmm_ensure_cow_for_range(dest_vaddr, length);
    uint8_t* dest = (uint8_t*)dest_vaddr;
    memcpy(dest, ksrc, length);

    if ((uint32_t)src < KERNEL_BASE) {
        kfree(ksrc);
    }

    vmm_switch_pdir(prev_pdir);
}

void vmm_zero_user_pages(pdirectory_t* pdir)
{
    for (int i = 0; i < VMM_KERNEL_TABLES_START; i++) {
        table_desc_t* ptable_desc = &pdir->entities[i];
        table_desc_del_attrs(ptable_desc, TABLE_DESC_WRITABLE);
        table_desc_set_attrs(ptable_desc, TABLE_DESC_ZEROING_ON_DEMAND);
    }
}

pdirectory_t* vmm_get_active_pdir()
{
    return _vmm_active_pdir;
}

pdirectory_t* vmm_get_kernel_pdir()
{
    return _vmm_kernel_pdir;
}

/**
 * PF HANDLER FUNCTIONS
 */

int vmm_tune_page(uint32_t vaddr, uint32_t settings)
{
    bool is_writable = ((settings & PAGE_WRITABLE) > 0);
    bool is_readable = ((settings & PAGE_READABLE) > 0);
    bool is_executable = ((settings & PAGE_EXECUTABLE) > 0);
    bool is_not_cacheable = ((settings & PAGE_NOT_CACHEABLE) > 0);
    bool is_cow = ((settings & PAGE_COW) > 0);
    bool is_user = ((settings & PAGE_USER) > 0);

    ptable_t* ptable = (ptable_t*)_vmm_pspace_get_vaddr_of_active_ptable(vaddr);
    page_desc_t* page = vmm_ptable_lookup(ptable, vaddr);

    if (page_desc_is_present(*page)) {
        is_user ? page_desc_set_attrs(page, PAGE_DESC_USER) : page_desc_del_attrs(page, PAGE_DESC_USER);
        is_writable ? page_desc_set_attrs(page, PAGE_DESC_WRITABLE) : page_desc_del_attrs(page, PAGE_DESC_WRITABLE);
        is_not_cacheable ? page_desc_set_attrs(page, PAGE_DESC_NOT_CACHEABLE) : page_desc_del_attrs(page, PAGE_DESC_NOT_CACHEABLE);
    } else {
        vmm_load_page(vaddr, settings);
    }

    system_flush_tlb_entry(vaddr);
    return 0;
}

int vmm_tune_pages(uint32_t vaddr, uint32_t length, uint32_t settings)
{
    uint32_t page_addr = PAGE_START(vaddr);
    while (page_addr < vaddr + length) {
        vmm_tune_page(page_addr, settings);
        page_addr += VMM_PAGE_SIZE;
    }
    return 0;
}

int vmm_load_page(uint32_t vaddr, uint32_t settings)
{
    uint32_t paddr = _vmm_alloc_page_paddr();
    if (!paddr) {
        /* TODO: Swap pages to make it able to allocate. */
        kpanic("NO PHYSICAL SPACE");
    }
    int res = vmm_map_page(vaddr, paddr, settings);
    uint8_t* dest = (uint8_t*)_vmm_round_floor_to_page(vaddr);
    memset(dest, 0, VMM_PAGE_SIZE);
    return res;
}

/**
 * The function is supposed to copy a page from @src_ptable to active
 * ptable. The page which is copied has address @vaddr. ONLY TO RESOLVE COW!
 */
int vmm_copy_page(uint32_t to_vaddr, uint32_t src_vaddr, ptable_t* src_ptable)
{
    page_desc_t* old_page_desc = vmm_ptable_lookup(src_ptable, src_vaddr);

    /* Based on an old page */
    ptable_t* cur_ptable = (ptable_t*)_vmm_pspace_get_vaddr_of_active_ptable(to_vaddr);
    page_desc_t* cur_page = vmm_ptable_lookup(cur_ptable, to_vaddr);

    if (!page_desc_is_present(*cur_page)) {
        vmm_load_page(to_vaddr, page_desc_get_settings_ignore_cow(*old_page_desc));
    } else {
        _vmm_ensure_cow_for_page(to_vaddr);
    }

    /* Mapping the old page to do a copy */
    zone_t tmp_zone = zoner_new_zone(VMM_PAGE_SIZE);
    uint32_t old_page_vaddr = (uint32_t)tmp_zone.start;
    uint32_t old_page_paddr = page_desc_get_frame(*old_page_desc);
    vmm_map_page(old_page_vaddr, old_page_paddr, PAGE_READABLE | PAGE_WRITABLE | PAGE_EXECUTABLE);

    memcpy((uint8_t*)to_vaddr, (uint8_t*)old_page_vaddr, VMM_PAGE_SIZE);

    /* Freeing */
    vmm_unmap_page(old_page_vaddr);
    zoner_free_zone(tmp_zone);
    return 0;
}

// currently unused and unoptimized with rewritten vmm
int vmm_alloc_page(page_desc_t* page)
{
    void* new_block = kmalloc_page_aligned();
    if (!new_block) {
        return -VMM_ERR_BAD_ADDR;
    }
    page_desc_set_attrs(page, PAGE_DESC_PRESENT);
    page_desc_set_frame(page, (uint32_t)new_block);
    return 0;
}

// currently unused and unoptimized with rewritten vmm
int vmm_free_page(page_desc_t* page)
{
    uint32_t frame = page_desc_get_frame(*page);
    pmm_free_block((void*)(frame * VMM_PAGE_SIZE));
    page_desc_del_attrs(page, PAGE_DESC_PRESENT);
    return 0;
}

int vmm_page_fault_handler(uint32_t info, uint32_t vaddr)
{
    if (_vmm_is_table_not_present(info) || _vmm_is_page_not_present(info)) {
        if (PAGE_CHOOSE_OWNER(vaddr) == PAGE_USER && vmm_get_active_pdir() != vmm_get_kernel_pdir()) {
            proc_t* holder_proc = tasking_get_proc_by_pdir(vmm_get_active_pdir());
            if (!holder_proc) {
                kpanic("No proc with the pdir\n");
            }

            proc_zone_t* zone = proc_find_zone(holder_proc, vaddr);
            if (!zone) {
                return SHOULD_CRASH;
            }

#ifdef VMM_DEBUG
            log("Mmap page %x for %d pid: %x", vaddr, RUNNIG_THREAD->process->pid, zone->flags);
#endif
            vmm_load_page(vaddr, zone->flags);

            if (zone->type == ZONE_TYPE_MAPPED_FILE_PRIVATLY) {
                zone->fd->offset = PAGE_START(vaddr) - zone->start;
                vfs_read(zone->fd, (void*)PAGE_START(vaddr), VMM_PAGE_SIZE);
            }
        } else {
            /* FIXME: Now we have a standard zone for kernel, but it's better to do the same thing as for user's pages */
            vmm_load_page(vaddr, PAGE_READABLE | PAGE_WRITABLE | PAGE_EXECUTABLE);
        }
        return OK;
    }

    if (_vmm_is_caused_writing(info)) {
        int visited = 0;
        if (_vmm_is_copy_on_write(vaddr)) {
            proc_t* holder_proc = tasking_get_proc_by_pdir(vmm_get_active_pdir());
            if (!holder_proc) {
                kpanic("No proc with the pdir\n");
            }
            _vmm_resolve_copy_on_write(holder_proc, vaddr);
            visited++;
        }
        // if (_vmm_is_zeroing_on_demand(vaddr)) {
        //     _vmm_resolve_zeroing_on_demand(vaddr);
        //     visited++;
        // }
        if (!visited) {
            ASSERT(false);
        }
    }

    return OK;
}

/**
 * CPU BASED FUNCTIONS
 */

int vmm_switch_pdir(pdirectory_t* pdir)
{
    if (!pdir) {
        return -1;
    }

    if (((uint32_t)pdir & (PDIR_SIZE - 1)) != 0) {
        kpanic("vmm_switch_pdir: wrong pdir");
    }

    system_disable_interrupts();
    if (_vmm_active_pdir == pdir) {
        system_enable_interrupts();
        return 0;
    }
    _vmm_active_pdir = pdir;
    system_set_pdir((uint32_t)_vmm_convert_vaddr2paddr((uint32_t)pdir));
    system_enable_interrupts();
    return 0;
}

void vmm_enable_paging()
{
    system_enable_paging();
}

void vmm_disable_paging()
{
    system_disable_paging();
}

/**
 * VM SELF TEST FUNCTIONS
 */

static int _vmm_self_test()
{
    vmm_map_pages(0x00f0000, 0x8f000000, 1, PAGE_READABLE | PAGE_WRITABLE | PAGE_EXECUTABLE);
    bool correct = true;
    correct &= ((uint32_t)_vmm_convert_vaddr2paddr(KERNEL_BASE) == KERNEL_PM_BASE);
    correct &= ((uint32_t)_vmm_convert_vaddr2paddr(0xffc00000) == 0x0);
    correct &= ((uint32_t)_vmm_convert_vaddr2paddr(0x100) == 0x100);
    correct &= ((uint32_t)_vmm_convert_vaddr2paddr(0x8f000000) == 0x00f0000);
    if (correct) {
        return 0;
    }
    return 1;
}

static bool vmm_test_pspace_vaddr_of_active_ptable()
{
    uint32_t vaddr = 0xc0000000;
    ptable_t* pt = _vmm_pspace_get_vaddr_of_active_ptable(vaddr);
    page_desc_t* ppage = vmm_ptable_lookup(pt, vaddr);
    page_desc_del_attrs(ppage, PAGE_DESC_PRESENT);
    uint32_t* kek1 = (uint32_t*)vaddr;
    *kek1 = 1;
    // should cause PF
    while (1) {
    }
    return true;
}
