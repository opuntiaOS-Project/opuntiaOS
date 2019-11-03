#include <mem/vmm/vmm.h>
#include <drivers/display.h>

#define VMM_OFFSET_IN_DIRECTORY(a) (((a) >> 22) & 0x3ff)
#define VMM_OFFSET_IN_TABLE(a) (((a) >> 12) & 0x3ff)
#define VMM_OFFSET_IN_PAGE(a) ((a) & 0x1000;

pdirectory_t* _vmm_cur_pdir = 0;

bool vmm_init() {
    ptable_t* table_0mb = (ptable_t*)pmm_alloc_block();
    ptable_t* table_3gb = (ptable_t*)pmm_alloc_block();
    ptable_t* table_stack = (ptable_t*)pmm_alloc_block();
    pdirectory_t* dir = (pdirectory_t*)pmm_alloc_block();
    if (!table_3gb || !table_0mb || !dir) {
        return false;
    }

    for (uint32_t phyz = 0, virt = 0, i = 0; i < 1024; phyz+=VMM_PAGE_SIZE, virt += VMM_PAGE_SIZE, i++) {
        pte_t new_page = 0;
        pte_set_attr(&new_page, PTE_PRESENT);
        pte_set_attr(&new_page, PTE_WRITABLE);
        pte_set_frame(&new_page, phyz / VMM_PAGE_SIZE);
        table_0mb->entities[VMM_OFFSET_IN_TABLE(virt)] = new_page;
    }


    for (uint32_t phyz = 0x100000, virt = 0xc0000000, i = 0; i < 1024; phyz+=VMM_PAGE_SIZE, virt += VMM_PAGE_SIZE, i++) {
        pte_t new_page = 0;
        pte_set_attr(&new_page, PTE_PRESENT);
        pte_set_attr(&new_page, PTE_WRITABLE);
        pte_set_frame(&new_page, phyz / VMM_PAGE_SIZE);
        table_3gb->entities[VMM_OFFSET_IN_TABLE(virt)] = new_page;
    }

    // map 4mb - to the last 4mb
    for (uint32_t phyz = 0x000000, virt = 0xffc00000, i = 0; i < 1024; phyz+=VMM_PAGE_SIZE, virt += VMM_PAGE_SIZE, i++) {
        pte_t new_page = 0;
        pte_set_attr(&new_page, PTE_PRESENT);
        pte_set_attr(&new_page, PTE_WRITABLE);
        pte_set_frame(&new_page, phyz / VMM_PAGE_SIZE);
        table_stack->entities[VMM_OFFSET_IN_TABLE(virt)] = new_page;
    }

    // memset(new_table, 0, sizeof(ptable_t));
    for (int i = 0; i < 1024; i++) {
        dir->entities[i] = 0;
    }

    uint32_t table_0mb_int = table_0mb;
    pde_t *pde_0mb = vmm_pdirectory_lookup(dir, 0x0);
    pde_set_attr(pde_0mb, PDE_PRESENT);
    pde_set_attr(pde_0mb, PDE_WRITABLE);
    pde_set_frame(pde_0mb, table_0mb_int / VMM_PAGE_SIZE);

    uint32_t table_3gb_int = table_3gb;
    pde_t *pde_3gb = vmm_pdirectory_lookup(dir, 0xc0000000);
    pde_set_attr(pde_3gb, PDE_PRESENT);
    pde_set_attr(pde_3gb, PDE_WRITABLE);
    pde_set_frame(pde_3gb, table_3gb_int / VMM_PAGE_SIZE);

    uint32_t table_stack_int = table_stack;
    pde_t *pde_stack = vmm_pdirectory_lookup(dir, 0xffc00000);
    pde_set_attr(pde_stack, PDE_PRESENT);
    pde_set_attr(pde_stack, PDE_WRITABLE);
    pde_set_frame(pde_stack, table_stack_int / VMM_PAGE_SIZE);

    vmm_switch_pdir(dir);

    vmm_enable_paging(true);
}

bool vmm_load_page(uint32_t t_vert) {
    uint32_t new_page_phyz_addr = pmm_alloc_block();
    return vmm_map_page(new_page_phyz_addr, t_vert);
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

bool vmm_map_page(uint32_t t_phyz, uint32_t t_virt) {
    pdirectory_t* cur_pdir = vmm_get_current_pdir();
    if (!cur_pdir) {
        return false;
    }
    pde_t* cur_ptable = vmm_pdirectory_lookup(cur_pdir, t_virt);
    // printh(*cur_ptable);
    if (!(*cur_ptable & 1)) {

        printf("Creating Table");

        ptable_t* new_table = (ptable_t*)pmm_alloc_block();

        if (!new_table) {
            return false;
        }

        // memset(new_table, 0, sizeof(ptable_t));
        uint8_t* new_table_for_memset = (uint8_t*)new_table;
        for (int i = 0; i < sizeof(ptable_t); i++) {
            new_table_for_memset[i] = 0;
        }

        uint32_t new_table_int = new_table;
        pde_t* new_table_pde = vmm_pdirectory_lookup(cur_pdir, t_virt);
        pde_set_attr(new_table_pde, PDE_PRESENT);
        pde_set_attr(new_table_pde, PDE_WRITABLE);
        pde_set_frame(new_table_pde, new_table_int / VMM_PAGE_SIZE);
    }
    cur_ptable = vmm_pdirectory_lookup(cur_pdir, t_virt);
    ptable_t *phyz_table_address = (ptable_t*)(pde_get_frame(*cur_ptable) * VMM_PAGE_SIZE);
    pte_t* page = vmm_ptable_lookup(phyz_table_address, t_virt);
    pte_set_attr(page, PTE_PRESENT);
    pte_set_frame(page, t_phyz / VMM_PAGE_SIZE);
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
    asm volatile ("cli");
    asm volatile ("mov %%eax, %%cr3" : : "a"(t_pdir));
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
    // while(1) {}
}
