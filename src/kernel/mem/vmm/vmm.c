#include <mem/vmm/vmm.h>
#include <drivers/display.h>
#include <global.h>
#include <tasking/tasking.h>
#include <mem/malloc.h>
#include <utils/kernel_self_test.h>

#define pdir_t pdirectory_t
#define VMM_OFFSET_IN_DIRECTORY(a) (((a) >> 22) & 0x3ff)
#define VMM_OFFSET_IN_TABLE(a) (((a) >> 12) & 0x3ff)
#define VMM_OFFSET_IN_PAGE(a) ((a) & 0xfff);

#define VMM_KERNEL_TABLES_START 768
#define VMM_USER_TABLES_START 0
#define IS_INDIVIDUAL_PER_DIR(index) (index < VMM_KERNEL_TABLES_START || (index == VMM_OFFSET_IN_DIRECTORY(pspace_start_vaddr)))

// owner of page or table
#define USER true
#define KERNEL false
#define CHOOSE_OWNER(vaddr) (vaddr >= KERNEL_BASE ? KERNEL : USER)

static pdir_t *_vmm_kernel_pdir;
static pdir_t *_vmm_active_pdir;
static uint32_t pspace_start_vaddr; 
static uint32_t kernel_ptables_start_paddr; 
static uint32_t kmalloc_start_vaddr;

#define vmm_kernel_pdir_phys2virt(paddr) ((void*)((uint32_t)paddr + KERNEL_BASE - KERNEL_PM_BASE))


/**
 * PRIVATE FUNCTIONS
 */

inline static void* _vmm_pspace_get_vaddr_of_active_pdir();
inline static void* _vmm_pspace_get_nth_active_ptable(uint32_t n);
inline static void* _vmm_pspace_get_vaddr_of_active_ptable(uint32_t vaddr);
static bool _vmm_split_pspace();
static void _vmm_pspace_init();
static table_desc_t _vmm_pspace_gen();
static void* _vmm_kernel_convert_vaddr2paddr(uint32_t vaddr);
static void* _vmm_convert_vaddr2paddr(uint32_t vaddr);

inline static void* _vmm_alloc_kernel_block();
static bool _vmm_init_switch_to_kernel_pdir();
static void _vmm_map_init_kernel_pages(uint32_t paddr, uint32_t vaddr);
static bool _vmm_create_kernel_ptables();
static bool _vmm_map_kernel();

inline static void _vmm_flush_tlb_entry(uint32_t vaddr);

static bool _vmm_is_copy_on_write(uint32_t vaddr);
static void _vmm_resolve_copy_on_write(uint32_t vaddr);

static int _vmm_self_test();


/**
 * PSPACE FUNCTIONS
 * 
 * Pspace is a space with all current tables mapped there.
 * The space starts from @pspace_start_vaddr and length is 4mb
 */

/**
 * The function is supposed to setup pspace_start_vaddr
 * Every virtual space has its own user's ptables in the area.
 * The kernel's patables are the same.
 */

inline static void* _vmm_pspace_get_vaddr_of_active_pdir() {
    return (void*)_vmm_active_pdir;
}

inline static void* _vmm_pspace_get_nth_active_ptable(uint32_t n) {
    return (void*)(pspace_start_vaddr + n * VMM_PAGE_SIZE);
}

inline static void* _vmm_pspace_get_vaddr_of_active_ptable(uint32_t vaddr) {
    return (void*)_vmm_pspace_get_nth_active_ptable(VMM_OFFSET_IN_DIRECTORY(vaddr));
}

static bool _vmm_split_pspace() {
    pspace_start_vaddr = 0xc0400000;
    
    if (VMM_OFFSET_IN_TABLE(pspace_start_vaddr) != 0) {
        kpanic("WRONG PSPACE START ADDR");
    }
    
    // TODO fix here
    kernel_ptables_start_paddr = 0x220000;
    while ((uint32_t)pmm_alloc_block() < (kernel_ptables_start_paddr - 2 * VMM_PAGE_SIZE)) {}
    _vmm_kernel_pdir = (pdir_t*)_vmm_alloc_kernel_block();
    _vmm_active_pdir = (pdir_t*)_vmm_kernel_pdir;
    memset((void*)_vmm_active_pdir, 0, sizeof(*_vmm_active_pdir)); // TODO problem for now
    kmalloc_start_vaddr = pspace_start_vaddr + 4 * 1024 * 1024; // plus 4mb
}

/**
 * The function is used to init pspace
 * Used only in the first stage of VM init
 */
static void _vmm_pspace_init() {
    uint32_t kernel_ptabels_vaddr = pspace_start_vaddr + 768 * VMM_PAGE_SIZE; // map what
    uint32_t kernel_ptabels_paddr = kernel_ptables_start_paddr;  // map to
    for (int i = 768; i < 1024; i++) {
        table_desc_t *ptable_desc = vmm_pdirectory_lookup(_vmm_active_pdir, kernel_ptabels_vaddr);
        if (!table_desc_is_present(*ptable_desc)) {
            // must present
            kpanic("PSPACE_6335 : BUG\n");
        }
        ptable_t *ptable_vaddr = (ptable_t*)(kernel_ptables_start_paddr + (VMM_OFFSET_IN_DIRECTORY(kernel_ptabels_vaddr) - 768) * VMM_PAGE_SIZE);
        page_desc_t* page = vmm_ptable_lookup(ptable_vaddr, kernel_ptabels_vaddr);
        page_desc_set_attr(page, PAGE_DESC_PRESENT);
        page_desc_set_attr(page, PAGE_DESC_WRITABLE);
        page_desc_set_frame(page, kernel_ptabels_paddr / VMM_PAGE_SIZE);
        kernel_ptabels_vaddr += VMM_PAGE_SIZE;
        kernel_ptabels_paddr += VMM_PAGE_SIZE;
    }
}

/**
 * The function is used to generate new pspace.
 * The function returns the table of itself. 
 */
static table_desc_t _vmm_pspace_gen() {
    ptable_t *cur_ptable = (ptable_t*)_vmm_pspace_get_nth_active_ptable(769);
    ptable_t *new_ptable = (ptable_t*)kmalloc_full_page();
    for (int i = 0; i < 1024; i++) {
        // coping all pages
        new_ptable->entities[i] = cur_ptable->entities[i];
    }

    page_desc_t pspace_page = 0;
    page_desc_set_attr(&pspace_page, PAGE_DESC_PRESENT);
    page_desc_set_attr(&pspace_page, PAGE_DESC_WRITABLE);
    page_desc_set_frame(&pspace_page, (uint32_t)_vmm_convert_vaddr2paddr((uint32_t)new_ptable) / VMM_PAGE_SIZE);
    new_ptable->entities[VMM_OFFSET_IN_DIRECTORY(pspace_start_vaddr)] = pspace_page;
    
    table_desc_t pspace_table = 0;
    table_desc_set_attr(&pspace_table, TABLE_DESC_PRESENT);
    table_desc_set_attr(&pspace_table, TABLE_DESC_WRITABLE);
    table_desc_set_frame(&pspace_table, (uint32_t)_vmm_convert_vaddr2paddr((uint32_t)new_ptable) / VMM_PAGE_SIZE);

    return pspace_table; 
}

/**
 * The function is used to traslate a virtual address into physical
 * Used only in the first stage of VM init
 */
static void* _vmm_kernel_convert_vaddr2paddr(uint32_t vaddr) {
    ptable_t *ptable_vaddr = (ptable_t*)(kernel_ptables_start_paddr + (VMM_OFFSET_IN_DIRECTORY(vaddr) - 768) * VMM_PAGE_SIZE);
    page_desc_t *page_desc = vmm_ptable_lookup(ptable_vaddr, vaddr);
    return (void*)((page_desc_get_frame(*page_desc)) | (vaddr & 0xfff));
}

/**
 * The function is used to traslate a virtual address into physical
 */
static void* _vmm_convert_vaddr2paddr(uint32_t vaddr) {
    ptable_t *ptable_vaddr = (ptable_t*)_vmm_pspace_get_vaddr_of_active_ptable(vaddr);
    page_desc_t *page_desc = vmm_ptable_lookup(ptable_vaddr, vaddr);
    return (void*)((page_desc_get_frame(*page_desc)) | (vaddr & 0xfff));
}


/**
 * VM INITIALIZATION FUNCTIONS
 */

inline static void* _vmm_alloc_kernel_block() {
    return vmm_kernel_pdir_phys2virt(pmm_alloc_block());
}

/**
 * The function is used to update active pdir
 * Used only in the first stage of VM init
 */
static bool _vmm_init_switch_to_kernel_pdir() {
    _vmm_active_pdir = _vmm_kernel_pdir;
    asm volatile ("cli");
    asm volatile ("mov %%eax, %%cr3" : : "a"((uint32_t)_vmm_kernel_convert_vaddr2paddr((uint32_t)_vmm_active_pdir)));
    asm volatile ("sti");
    return true;
}

/**
 * The function is used to map kernel pages
 * Used only in the first stage of VM init
 */
static void _vmm_map_init_kernel_pages(uint32_t paddr, uint32_t vaddr) {
    ptable_t *ptable_paddr = (ptable_t*)(kernel_ptables_start_paddr + (VMM_OFFSET_IN_DIRECTORY(vaddr) - 768) * VMM_PAGE_SIZE);
    for (uint32_t phyz = paddr, virt = vaddr, i = 0; i < 1024; phyz+=VMM_PAGE_SIZE, virt += VMM_PAGE_SIZE, i++) {
        pte_t new_page = ((phyz / VMM_PAGE_SIZE) << 12) | 3;
        ptable_paddr->entities[i] = new_page;
    }
}

/**
 * The function is supposed to create all kernel tables and map necessary
 * data into _vmm_kernel_pdir.
 * Used only in the first stage of VM init
 */
static bool _vmm_create_kernel_ptables() {
    uint32_t table_coverage = VMM_PAGE_SIZE * 1024;
    uint32_t kernel_ptabels_vaddr = VMM_KERNEL_TABLES_START * table_coverage;
    
    for (int i = VMM_KERNEL_TABLES_START; i < 1024; i++, kernel_ptabels_vaddr += table_coverage) {        
        // updating table descriptor of kernel's pdir
        table_desc_t* ptable_desc = vmm_pdirectory_lookup(_vmm_kernel_pdir, kernel_ptabels_vaddr);
        uint32_t paddr = (uint32_t)pmm_alloc_block();
        if (!paddr) {
            kpanic("PADDR_5546 : BUG\n");
        }
        table_desc_set_attr(ptable_desc, TABLE_DESC_PRESENT);
        table_desc_set_attr(ptable_desc, TABLE_DESC_WRITABLE);
        table_desc_set_frame(ptable_desc, paddr / VMM_PAGE_SIZE);
    }
    
    _vmm_map_init_kernel_pages(0x00100000, 0xc0000000);
    _vmm_map_init_kernel_pages(0x00000000, 0xffc00000);

    return true;
}

/**
 * The function is supposed to map secondary data for kernel's pdir.
 * Used only in the first stage of VM init
 */
static bool _vmm_map_kernel() {
    vmm_map_pages(0x00000000, 0x00000000, 1024, KERNEL);
    return true;
}

int vmm_setup() {
    _vmm_split_pspace();
    _vmm_create_kernel_ptables();
    _vmm_pspace_init();
    _vmm_init_switch_to_kernel_pdir();
    _vmm_map_kernel();
    kmalloc_init(kmalloc_start_vaddr);
    if (_vmm_self_test() < 0) {
        kpanic("VMM SELF TEST Not passed");
    }
    return 0;
}

/**
 * VM TOOLS
 */

inline static void _vmm_flush_tlb_entry(uint32_t vaddr) {
    asm volatile("invlpg (%0)" ::"r" (vaddr) : "memory");
}

table_desc_t* vmm_pdirectory_lookup(pdirectory_t *pdir, uint32_t vaddr) {
    if (pdir) {
        return &pdir->entities[VMM_OFFSET_IN_DIRECTORY(vaddr)];
    }
    return 0;
}

page_desc_t* vmm_ptable_lookup(ptable_t *ptable, uint32_t vaddr) {
    if (ptable) {
        return &ptable->entities[VMM_OFFSET_IN_TABLE(vaddr)];
    }
    return 0;
}

/**
 * The function is supposed to create new ptable (not kernel) and
 * rebuild pspace to match to the new setup
 */
int vmm_allocate_ptable(uint32_t vaddr) {
    if (_vmm_active_pdir == 0) {
        return -VMM_ERR_PDIR; 
    }

    uint32_t ptable_paddr = (uint32_t)pmm_alloc_block();
    if (!ptable_paddr) {
        // TODO may be cleaner should be called here;
        return -VMM_ERR_NO_SPACE;
    }
    uint32_t ptable_vaddr = (uint32_t)_vmm_pspace_get_vaddr_of_active_ptable(vaddr);

    table_desc_t *ptable_desc = vmm_pdirectory_lookup(_vmm_active_pdir, vaddr);
    table_desc_set_attr(ptable_desc, TABLE_DESC_PRESENT);
    table_desc_set_attr(ptable_desc, TABLE_DESC_WRITABLE);
    table_desc_set_frame(ptable_desc, ptable_paddr / VMM_PAGE_SIZE);

    if (CHOOSE_OWNER(vaddr) == USER) {
        table_desc_set_attr(ptable_desc, TABLE_DESC_USER);
    }
    
    return vmm_map_page(ptable_vaddr, ptable_paddr, CHOOSE_OWNER(vaddr));
}

/**
 * The function is supposed to map vaddr to paddr
 */
int vmm_map_page(uint32_t vaddr, uint32_t paddr, bool owner) {
    if (!_vmm_active_pdir) {
        return -VMM_ERR_PDIR; 
    }

    table_desc_t *ptable_desc = vmm_pdirectory_lookup(_vmm_active_pdir, vaddr);
    if (!table_desc_is_present(*ptable_desc)) {
        vmm_allocate_ptable(vaddr);
    }

    ptable_t* ptable = (ptable_t*)_vmm_pspace_get_vaddr_of_active_ptable(vaddr);
    page_desc_t* page = vmm_ptable_lookup(ptable, vaddr);
    page_desc_set_attr(page, PAGE_DESC_PRESENT);
    page_desc_set_attr(page, PAGE_DESC_WRITABLE);
    page_desc_set_frame(page, paddr / VMM_PAGE_SIZE);
    
    if (owner == USER) {
        page_desc_set_attr(page, PAGE_DESC_USER);
    }
    
    _vmm_flush_tlb_entry(vaddr);

    return 0;
}

/**
 * The function is supposed to map a sequence of vaddrs to paddrs
 */
int vmm_map_pages(uint32_t vaddr, uint32_t paddr, uint32_t n_pages, bool owner) {
    if ((paddr & 0xfff) || (vaddr & 0xfff)) {
        return -VMM_ERR_BAD_ADDR;
    }

    int status = 0;
    for (; n_pages; paddr+=VMM_PAGE_SIZE, vaddr+=VMM_PAGE_SIZE, n_pages--) {
        if (status = vmm_map_page(vaddr, paddr, owner) < 0) {
            return status;
        }
    }

    return 0;
}


/**
 * COPY ON WRITE FUNCTIONS
 */

static bool _vmm_is_copy_on_write(uint32_t vaddr) {
    table_desc_t *ptable_desc = vmm_pdirectory_lookup(_vmm_active_pdir, vaddr);
    return table_desc_is_copy_on_write(*ptable_desc);
}

// TODO handle delete of tables and dirs
static void _vmm_resolve_copy_on_write(uint32_t vaddr) {
    table_desc_t *ptable_desc = vmm_pdirectory_lookup(_vmm_active_pdir, vaddr);
    ptable_t *src_ptable = kmalloc_full_page();
    ptable_t *root_ptable = _vmm_pspace_get_vaddr_of_active_ptable(vaddr);
    memcpy((uint8_t*)src_ptable, (uint8_t*)root_ptable, VMM_PAGE_SIZE);
    vmm_allocate_ptable(vaddr);
    ptable_t *new_ptable = _vmm_pspace_get_vaddr_of_active_ptable(vaddr);
    
    // currently do that for all pages
    for (int i = 0; i < 1024; i++) {
        uint32_t page_vaddr = ((vaddr >> 22) << 22) + (i * VMM_PAGE_SIZE);
        page_desc_t *page_desc = vmm_ptable_lookup(src_ptable, page_vaddr);
        if ((uint32_t)*page_desc != 0) {
            vmm_copy_page(page_vaddr, src_ptable);
        }
    }
}

/**
 * The function is supposed to copy a page from @src_ptable to active
 * ptable. The page which is copied has address @vaddr.
 */
int vmm_copy_page(uint32_t vaddr, ptable_t *src_ptable) {
    vmm_load_page(vaddr);
    uint32_t old_page_vaddr = (uint32_t)kmalloc_full_page_only_vaddr();
    page_desc_t *old_page_desc = vmm_ptable_lookup(src_ptable, vaddr);
    uint32_t old_page_paddr = page_desc_get_frame(*old_page_desc);
    vmm_map_page(old_page_vaddr, old_page_paddr, KERNEL);
    memcpy((uint8_t*)vaddr, (uint8_t*)old_page_vaddr, VMM_PAGE_SIZE);
    // TODO free address only;
    // vmm_unmap_page(old_page_vaddr, old_page_paddr, KERNEL);
    // free();
    return 0;
}


/**
 * USER PDIR MANIPULATIONS FUNCTIONS
 */

// creating user pdir
// copy kernel's table to the table
// TODO may not work because of PHYS address translation (Bug #3)

/**
 * The function is supposed to create all new user's pdir.
 */
pdirectory_t* vmm_new_user_pdir() {
    pdirectory_t *pdir = (pdirectory_t*)kmalloc_full_page();
    
    for (int i = VMM_USER_TABLES_START; i < VMM_KERNEL_TABLES_START; i++) {
        pdir->entities[i] = 0;
    }
    
    for (int i = VMM_KERNEL_TABLES_START; i < 1024; i++) {
        if (!IS_INDIVIDUAL_PER_DIR(i)) {
            pdir->entities[i] = _vmm_active_pdir->entities[i];
        }
    }

    pdir->entities[VMM_OFFSET_IN_DIRECTORY(pspace_start_vaddr)] = _vmm_pspace_gen();
    return pdir;
}

/**
 * The function is supposed to fork a new user's pdir from the active
 */
pdirectory_t* vmm_new_forked_user_pdir() {
    pdirectory_t *pdir = (pdirectory_t*)kmalloc_full_page();
    
    // coping all tables
    for (int i = 0; i < 1024; i++) {
        pdir->entities[i] = _vmm_active_pdir->entities[i];
    }
    pdir->entities[VMM_OFFSET_IN_DIRECTORY(pspace_start_vaddr)] = _vmm_pspace_gen();
    
    for (int i = 0; i < VMM_KERNEL_TABLES_START; i++) {
        table_desc_t *ptable_desc = &pdir->entities[i];
        table_desc_del_attr(ptable_desc, TABLE_DESC_WRITABLE);
        table_desc_set_attr(ptable_desc, TABLE_DESC_COPY_ON_WRITE);
    }

    return pdir;
}

/** WILL BE DEPRECATED
 * The function is supposed to copy init program data into the pdir
 */
void vmm_copy_program_data(pdirectory_t* dir, uint8_t* data, uint32_t data_size) {
    if (data_size > 256) {
        kpanic("Init proccess is too big");
    }
    
    uint8_t *tmp_block = kmalloc_full_page();
    
    for (int i = 0; i < data_size; i++) {
        tmp_block[i] = data[i];
    }
    
    vmm_switch_pdir(dir);
    vmm_map_page(0x0, (uint32_t)_vmm_convert_vaddr2paddr((uint32_t)tmp_block), USER);
    vmm_switch_pdir(_vmm_kernel_pdir);
}

pdirectory_t* vmm_get_active_pdir() {
    return _vmm_active_pdir;
}

/**
 * PF HANDLER FUNCTIONS
 */

int vmm_load_page(uint32_t vaddr) {
    uint32_t paddr = (uint32_t)pmm_alloc_block();
    if (!paddr) {
        // clean here to make it able to allocate
        kpanic("NO SPACE");
    }
    return vmm_map_page(vaddr, paddr, CHOOSE_OWNER(vaddr));
}

// currently unused and unoptimized with rewritten vmm
int vmm_alloc_page(page_desc_t* page) {
    void* new_block = kmalloc_full_page();
    if (!new_block) {
        return -VMM_ERR_BAD_ADDR;
    }
    page_desc_set_attr(page, PAGE_DESC_PRESENT);
    page_desc_set_frame(page, (uint32_t)new_block / VMM_PAGE_SIZE);
    return 0;
}

// currently unused and unoptimized with rewritten vmm
int vmm_free_page(page_desc_t* page) {
    uint32_t frame = page_desc_get_frame(*page);
    pmm_free_block((void*)(frame * VMM_PAGE_SIZE));
    page_desc_del_attr(page, PAGE_DESC_PRESENT);
    return 0;
}

void vmm_page_fault_handler(uint8_t info, uint32_t vaddr) {
    // page doesn't present in memory
    if (info == 7) {
        // copy on write ?
        if (_vmm_is_copy_on_write(vaddr)) {
            // printf("COW\n");
            _vmm_resolve_copy_on_write(vaddr);
        } else {
            printf("NO_COW UNH\n");
            while (1) {}
        }
    } else {
        if ((info & 1) == 0) {
            // let's load page
            printf("Loading page: ");
            printh(vaddr);
            printf("\n");
            vmm_load_page(vaddr);
        } else {
            while (1) {}
        }
    }
}


/**
 * VM SETTING FUNCTIONS
 */

int vmm_switch_pdir(pdirectory_t *pdir) {
    if (((uint32_t)pdir & (VMM_PAGE_SIZE-1)) != 0) {
        kpanic("vmm_switch_pdir: wrong pdir");
    }

    _vmm_active_pdir = pdir;
    asm volatile ("cli");
    asm volatile ("mov %%eax, %%cr3" : : "a"((uint32_t)_vmm_convert_vaddr2paddr((uint32_t)pdir)));
    asm volatile ("sti");
    return 0;
}

void vmm_enable_paging() {
    asm volatile ("mov %cr0, %eax");
    asm volatile ("or $0x80000000, %eax");
    asm volatile ("mov %eax, %cr0");
}

void vmm_disable_paging() {
    asm volatile ("mov %cr0, %eax");
    asm volatile ("and $0x7FFFFFFF, %eax");
    asm volatile ("mov %eax, %cr0");
}


/**
 * VM SELF TEST FUNCTIONS
 */

static int _vmm_self_test() {
    vmm_map_pages(0x00f0000, 0x8f000000, 1, false);
    vmm_map_pages(0x00f0000, 0x8f000000, 1, false);
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

static bool vmm_test_pspace_vaddr_of_active_ptable() {
    uint32_t vaddr = 0xc0000000;
    ptable_t *pt = _vmm_pspace_get_vaddr_of_active_ptable(vaddr);
    page_desc_t *ppage = vmm_ptable_lookup(pt, vaddr);
    page_desc_del_attr(ppage, PAGE_DESC_PRESENT);
    uint32_t* kek1 = (uint32_t*)vaddr;
    *kek1 = 1;
    // should cause PF
    while (1) {}
    return true;
}
