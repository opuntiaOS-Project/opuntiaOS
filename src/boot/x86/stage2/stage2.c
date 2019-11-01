#include <types.h>
#include <drivers/ata.h>

typedef struct {
    uint16_t memory_map_size;
    uint16_t kernel_size;
} mem_desc_t;

#define VMM_PTE_COUNT (1024)
#define VMM_PDE_COUNT (1024)
#define VMM_PAGE_SIZE (4096)
#define VMM_OFFSET_IN_DIRECTORY(a) (((a) >> 22) & 0x3ff)
#define VMM_OFFSET_IN_TABLE(a) (((a) >> 12) & 0x3ff)
#define VMM_OFFSET_IN_PAGE(a) ((a) & 0x1000;
#define pte_t uint32_t
#define pde_t uint32_t

typedef struct {
    pte_t entities[VMM_PTE_COUNT];
} ptable_t;

typedef struct pdirectory {
    pde_t entities[VMM_PDE_COUNT];
} pdirectory_t;


void stage2(mem_desc_t *mem_desc) {
    ata_t ata0m;
    init_ata(&ata0m, 0x1F0, 1);
    indentify_ata_device(&ata0m);
    uint32_t place_to = 0x100000;
    mem_desc->kernel_size = ata_read_to_ram(&ata0m, 0, place_to, 2);
    place_to += 512 - 2;
    for (uint16_t i = 1; i < mem_desc->kernel_size * 2; i++) {
        ata_read_to_ram(&ata0m, i, place_to, 0);
        place_to += 512;
    }

    ptable_t* table_0mb = 0x9B000;
    ptable_t* table_3gb = 0x9C000;
    pdirectory_t* dir = 0x9D000;

    for (uint32_t phyz = 0, virt = 0, i = 0; i < 1024; phyz+=VMM_PAGE_SIZE, virt += VMM_PAGE_SIZE, i++) {
        pte_t new_page = 0;
        // pte_set_attr(&new_page, PTE_PRESENT);
        new_page |= 3;
        new_page |= ((phyz / VMM_PAGE_SIZE) << 12);
        table_0mb->entities[i] = new_page;
    }

    for (uint32_t phyz = 0x100000, virt = 0xc0000000, i = 0; i < 1024; phyz+=VMM_PAGE_SIZE, virt += VMM_PAGE_SIZE, i++) {
        pte_t new_page = 0;
        new_page |= 3;
        new_page |= ((phyz / VMM_PAGE_SIZE) << 12);
        table_3gb->entities[i] = new_page;
    }

    uint8_t* dir_for_memset = (uint8_t*)dir;
    for (int i = 0; i < 1024; i++) {
        dir->entities[i] = 0;
    }

    uint32_t table_0mb_int = table_0mb;
    dir->entities[0] |= 3;
    dir->entities[0] |= ((table_0mb_int / VMM_PAGE_SIZE) << 12);

    uint32_t table_3gb_int = table_3gb;
    dir->entities[768] |= 3;
    dir->entities[768] |= ((table_3gb_int / VMM_PAGE_SIZE) << 12);

    asm volatile ("mov %%eax, %%cr3" : : "a"(dir));
    // enabling paging
    asm volatile ("mov %cr0, %eax");
    asm volatile ("or $0x80000000, %eax");
    asm volatile ("mov %eax, %cr0");

    asm volatile("push %0" : : "r"(mem_desc));
    asm volatile("mov $0xc0000000, %eax");
    asm volatile("call %eax");
    while (1) {}
}
