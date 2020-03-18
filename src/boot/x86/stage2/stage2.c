#include <types.h>
#include <drivers/ata.h>

extern void init_ata(ata_t *ata, uint32_t port, char is_master);
extern void indentify_ata_device(ata_t *ata);
extern uint16_t ata_read_to_ram(ata_t *dev, uint32_t sectorNum, uint32_t toRam, uint8_t offset);

void stage2(mem_desc_t *mem_desc) {
    ata_t ata0m;
    init_ata(&ata0m, 0x1F0, 1);
    indentify_ata_device(&ata0m);
    uint32_t place_to = 0x100000;
    mem_desc->kernel_size = ata_read_to_ram(&ata0m, 1, place_to, 2);
    place_to += 512 - 2;
    for (uint16_t i = 1; i < mem_desc->kernel_size * 2; i++) {
        ata_read_to_ram(&ata0m, i+1, place_to, 0);
        place_to += 512;
    }

    ptable_t* table_0mb = (ptable_t*)0x9A000;
    ptable_t* table_0mbplus = (ptable_t*)0x98000;
    ptable_t* table_3gb = (ptable_t*)0x9B000;
    ptable_t* table_3gbplus = (ptable_t*)0x99000;
    ptable_t* table_stack = (ptable_t*)0x9C000;
    pdirectory_t* dir = (pdirectory_t*)0x9D000;

    for (uint32_t phyz = 0, virt = 0, i = 0; i < 1024; phyz+=VMM_PAGE_SIZE, virt += VMM_PAGE_SIZE, i++) {
        pte_t new_page = 0;
        // pte_set_attr(&new_page, PTE_PRESENT);
        new_page |= 3;
        new_page |= ((phyz / VMM_PAGE_SIZE) << 12);
        table_0mb->entities[i] = new_page;
    }

    for (uint32_t phyz = 0x400000, virt = 0x400000, i = 0; i < 1024; phyz+=VMM_PAGE_SIZE, virt += VMM_PAGE_SIZE, i++) {
        pte_t new_page = 0;
        // pte_set_attr(&new_page, PTE_PRESENT);
        new_page |= 3;
        new_page |= ((phyz / VMM_PAGE_SIZE) << 12);
        table_0mbplus->entities[i] = new_page;
    }

    for (uint32_t phyz = 0x100000, virt = 0xc0000000, i = 0; i < 1024; phyz+=VMM_PAGE_SIZE, virt += VMM_PAGE_SIZE, i++) {
        pte_t new_page = 0;
        new_page |= 3;
        new_page |= ((phyz / VMM_PAGE_SIZE) << 12);
        table_3gb->entities[i] = new_page;
    }

    for (uint32_t phyz = 0x500000, virt = 0xc0400000, i = 0; i < 1024; phyz+=VMM_PAGE_SIZE, virt += VMM_PAGE_SIZE, i++) {
        pte_t new_page = 0;
        new_page |= 3;
        new_page |= ((phyz / VMM_PAGE_SIZE) << 12);
        table_3gbplus->entities[i] = new_page;
    }

    for (uint32_t phyz = 0x000000, virt = 0xffc00000, i = 0; i < 1024; phyz+=VMM_PAGE_SIZE, virt += VMM_PAGE_SIZE, i++) {
        pte_t new_page = 0;
        new_page |= 3;
        new_page |= ((phyz / VMM_PAGE_SIZE) << 12);
        table_stack->entities[i] = new_page;
    }

    uint8_t* dir_for_memset = (uint8_t*)dir;
    for (int i = 0; i < 1024; i++) {
        dir->entities[i] = 0;
    }

    uint32_t table_0mb_int = (uint32_t)table_0mb;
    dir->entities[0] |= 3;
    dir->entities[0] |= ((table_0mb_int / VMM_PAGE_SIZE) << 12);

    table_0mb_int = (uint32_t)table_0mbplus;
    dir->entities[1] |= 3;
    dir->entities[1] |= ((table_0mb_int / VMM_PAGE_SIZE) << 12);

    uint32_t table_3gb_int = (uint32_t)table_3gb;
    dir->entities[768] |= 3;
    dir->entities[768] |= ((table_3gb_int / VMM_PAGE_SIZE) << 12);

    table_3gb_int = (uint32_t)table_3gbplus;
    dir->entities[769] |= 3;
    dir->entities[769] |= ((table_3gb_int / VMM_PAGE_SIZE) << 12);

    uint32_t table_stack_int = (uint32_t)table_stack;
    dir->entities[1023] |= 3;
    dir->entities[1023] |= ((table_stack_int / VMM_PAGE_SIZE) << 12);

    asm volatile ("mov %%eax, %%cr3" : : "a"(dir));
    // enabling paging
    asm volatile ("mov %cr0, %eax");
    asm volatile ("or $0x80000000, %eax");
    asm volatile ("mov %eax, %cr0");

    asm volatile ("add $0xffc00000, %ebp");
    asm volatile ("add $0xffc00000, %esp");

    asm volatile("push %0" : : "r"(mem_desc));
    asm volatile("mov $0xc0000000, %eax");
    asm volatile("call %eax");
    while (1) {}
}
