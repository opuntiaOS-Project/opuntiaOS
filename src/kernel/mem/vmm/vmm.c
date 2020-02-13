#include <mem/vmm/vmm.h>
#include <drivers/display.h>
#include <global.h>

#define VMM_OFFSET_IN_DIRECTORY(a) (((a) >> 22) & 0x3ff)
#define VMM_OFFSET_IN_TABLE(a) (((a) >> 12) & 0x3ff)
#define VMM_OFFSET_IN_PAGE(a) ((a) & 0x1000;

#define V2P(addr) ((uint32_t)addr - KERNEL_BASE + KERNEL_PM_BASE)
#define P2V(addr) ((uint32_t)addr + KERNEL_BASE - KERNEL_PM_BASE)

static pdirectory_t* _vmm_cur_pdir = 0;
static pdirectory_t* _vmm_cur_pdir_phys = 0;
pdirectory_t* _vmm_kpdir = 0;

void* _vmm_alloc_block() {
    return P2V(pmm_alloc_block());
}

bool vmm_setup() {
    _vmm_kpdir = (pdirectory_t*)_vmm_alloc_block();
    if (!_vmm_kpdir) {
        return false;
    }

    memset(_vmm_kpdir, 0, sizeof(*_vmm_kpdir));
    vmm_create_kernel_tables();
    _vmm_cur_pdir = _vmm_kpdir;
    vmm_map_pages(0x100000, 0xc0000000, 1024); // kernel code+data is mapped here
    vmm_map_pages(0x000000, 0xffc00000, 1024); // 16 bits data is mapped here and init stack
    vmm_map_pages(0x000000, 0x000000, 1024); // 16 bits data is mapped here and init stack
    
    vmm_switch_pdir(_vmm_cur_pdir);
    vmm_enable_paging(true);
    return true;
}


// creating user pdir
// copy kernel's tabel to the table
// TODO may not work because of PHYS adderes translation (Bug #3)
pdirectory_t* vmm_new_user_pdir() {
    pdirectory_t *new_table = (pdirectory_t*)_vmm_alloc_block();
    for (int i = 768; i < 1024; i++) {
        new_table->entities[i] = _vmm_kpdir->entities[i];
    }
    return new_table;
}

void vmm_copy_program_data(pdirectory_t* dir, uint8_t* data, uint32_t data_size) {
    if (data_size > 256) {
        kpanic("Init proccess is too big");
    }
    uint8_t *tmp_block = pmm_alloc_block();
    for (int i = 0; i < data_size; i++) {
        tmp_block[i] = data[i];
    }
    vmm_map_page(tmp_block, 0, dir, true);
}

bool vmm_load_page(uint32_t t_vert) {
    uint32_t new_page_phyz_addr = pmm_alloc_block();
    return vmm_map_page(new_page_phyz_addr, t_vert, 0, false);
}

bool vmm_alloc_page(pte_t* t_page) {
    void* new_block = pmm_alloc_block();
    if (!new_block) {
        return false;
    }
    pte_set_attr(t_page, PTE_PRESENT);
    pte_set_frame(t_page, (uint32_t)new_block / VMM_PAGE_SIZE);
    return true;
}

bool vmm_free_page(pte_t* t_page) {
    uint32_t frame = pte_get_frame(t_page);
    pmm_free_block(frame * VMM_PAGE_SIZE);
    pte_del_attr(t_page, PTE_PRESENT);
}

void vmm_create_kernel_tables() {
    if (!_vmm_kpdir) {
        return false;
    }
    uint32_t table_coverage = VMM_PAGE_SIZE * 1024;
    uint32_t virt = KERNEL_BASE;
    for (; virt < 0xffc00000; virt += table_coverage) {
        pde_t* cur_ptable = vmm_pdirectory_lookup(_vmm_kpdir, virt);
        ptable_t* new_table = (ptable_t*)_vmm_alloc_block();
        if (!new_table) {
            return false;
        }
        memset(new_table, 0, sizeof(*new_table));
        
        uint32_t new_table_int = V2P(new_table);
        pde_set_attr(cur_ptable, PDE_PRESENT);
        pde_set_attr(cur_ptable, PDE_WRITABLE);
        pde_set_frame(cur_ptable, new_table_int / VMM_PAGE_SIZE);
    }
    {
        pde_t* cur_ptable = vmm_pdirectory_lookup(_vmm_kpdir, 0xffc00000);
        ptable_t* new_table = (ptable_t*)_vmm_alloc_block();
        if (!new_table) {
            return false;
        }
        memset(new_table, 0, sizeof(*new_table));
        
        uint32_t new_table_int = V2P(new_table);
        pde_set_attr(cur_ptable, PDE_PRESENT);
        pde_set_attr(cur_ptable, PDE_WRITABLE);
        pde_set_frame(cur_ptable, new_table_int / VMM_PAGE_SIZE);
    }
}

bool vmm_map_page(uint32_t t_phyz, uint32_t t_virt, pdirectory_t* cur_pdir, bool user) {
    if (!cur_pdir) {
        cur_pdir = vmm_get_current_pdir();
    }
    if (!cur_pdir) {
        return false;
    }
    
    pde_t* cur_ptable = vmm_pdirectory_lookup(cur_pdir, t_virt);
    if (!(*cur_ptable & 1)) {
        ptable_t* new_table = (ptable_t*)_vmm_alloc_block();
        if (!new_table) {
            return false;
        }
        memset(new_table, 0, sizeof(*new_table));
        
        uint32_t new_table_int = V2P(new_table);
        pde_set_attr(cur_ptable, PDE_PRESENT);
        pde_set_attr(cur_ptable, PDE_WRITABLE);
        if (user) {
            pde_set_attr(cur_ptable, PDE_USER);
        }
        pde_set_frame(cur_ptable, new_table_int / VMM_PAGE_SIZE);
    }

    ptable_t *phys_table_address = (ptable_t*)(pde_get_frame(*cur_ptable) * VMM_PAGE_SIZE);
    ptable_t *virt_table_address = P2V(phys_table_address);
    pte_t* page = vmm_ptable_lookup(virt_table_address, t_virt);
    pte_set_attr(page, PTE_PRESENT);
    pte_set_attr(page, PTE_WRITABLE);
    if (user) {
        pte_set_attr(page, PTE_USER);
    }
    pte_set_frame(page, t_phyz / VMM_PAGE_SIZE);
    return true;
}

// Map n pages of currently active pdir
bool vmm_map_pages(uint32_t phys, uint32_t virt, uint32_t n_pages) {
    if ((phys & 0xfff) || (virt & 0xfff)) {
        return false;
    }
    for (; n_pages; phys+=VMM_PAGE_SIZE, virt+=VMM_PAGE_SIZE, n_pages--) {
        vmm_map_page(phys, virt, 0, false);
    }
    return true;
}

pte_t* vmm_ptable_lookup(ptable_t *t_ptable, uint32_t t_addr) {
    if (t_ptable) {
        return &t_ptable->entities[VMM_OFFSET_IN_TABLE(t_addr)];
    }
    return 0;
}

pde_t* vmm_pdirectory_lookup(pdirectory_t *t_pdir, uint32_t t_addr) {
    if (t_pdir) {
        return &t_pdir->entities[VMM_OFFSET_IN_DIRECTORY(t_addr)];
    }
    return 0;
}

bool vmm_switch_pdir(pdirectory_t *t_pdir) {
    if (!t_pdir) {
        return false;
    }

    _vmm_cur_pdir = t_pdir;
    _vmm_cur_pdir_phys = V2P(t_pdir);
    asm volatile ("cli");
    asm volatile ("mov %%eax, %%cr3" : : "a"(_vmm_cur_pdir_phys));
    asm volatile ("sti");
    return true;
}

pdirectory_t* vmm_get_current_pdir() {
    return _vmm_cur_pdir;
}

void vmm_enable_paging(bool enable) {
    if (enable) {
        asm volatile ("mov %cr0, %eax");
        asm volatile ("or $0x80000000, %eax");
        asm volatile ("mov %eax, %cr0");
    } else {
        asm volatile ("mov %cr0, %eax");
        asm volatile ("and $0x7FFFFFFF, %eax");
        asm volatile ("mov %eax, %cr0");
    }
}

void vmm_page_fault_handler(uint8_t t_info, uint32_t t_virt) {
    // page doesn't present in memory
    if ((t_info & 1) == 0) {
        // let's load page
        printf("Loading page: ");
        printh(t_virt);
        printf("\n");

        vmm_load_page(t_virt);
    }
}
