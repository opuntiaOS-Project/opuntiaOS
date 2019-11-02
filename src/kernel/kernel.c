#include <x86/idt.h>
#include <x86/pci.h>
#include <types.h>
#include <drivers/driverManager.h>
#include <drivers/ata.h>
#include <drivers/display.h>
#include <drivers/timer.h>
#include <mem/pmm.h>
#include <mem/malloc.h>

#include <qemulog.h>

#define MEMORY_MAP_REGION 0xA00

typedef struct {
    uint32_t startLo;
    uint32_t startHi;
    uint32_t sizeLo;
    uint32_t sizeHi;
    uint32_t type;
    uint32_t acpi_3_0;
} memory_map_t;

typedef struct {
    uint16_t memory_map_size;
    uint16_t kernel_size;
} mem_desc_t;

void stage3(mem_desc_t *mem_desc) {
    asm volatile("cli");
    log("aStage 3 started\n");
    clean_screen();
    printf("HERE");
    idt_setup();
    asm volatile("sti");
    // init_timer();

    printf("Kernel size: "); printd(mem_desc->kernel_size);
    printf(" KB\nMemory map:\n");
    uint32_t ram_size = 0;
    memory_map_t *memory_map = (memory_map_t *)MEMORY_MAP_REGION;
    for (int i = 0; i < mem_desc->memory_map_size; i++) {
        if (memory_map[i].type == 1) {
            ram_size = memory_map[i].startLo + memory_map[i].sizeLo;
        }
    }

    printh(ram_size);

    pmm_init(0x100000, mem_desc->kernel_size, ram_size);

    for (int i = 0; i < mem_desc->memory_map_size; i++) {
        printh(memory_map[i].startLo); printf(" ");
        printd(memory_map[i].sizeLo); printf(" ");
        printh(memory_map[i].type); printf(" ");
        if (memory_map[i].type == 1) {
            pmm_init_region(memory_map[i].startLo, memory_map[i].sizeLo);
        }
        printf("\n");
    }

    pmm_deinit_mat(); // mat deinit
    pmm_deinit_region(0x0, 0x100000); // kernel stack deinit
    pmm_deinit_region(0x100000, mem_desc->kernel_size * 1024); // kernel deinit

    if (vmm_init()) {
        printf("\n\nVM Remapped\n\n");
    }

    // heap area right after kernel space
    // temp solution will change
    kmalloc_init(0xc0000000 + 0x400000);

    uint32_t* kek1 = (uint32_t*)kmalloc(sizeof(uint32_t));
    uint32_t* kek2 = (uint32_t*)kmalloc(sizeof(uint32_t));
    *kek1 = 1;
    *kek2 = 2;

    int* newpage = (int *)0x10000000;
    // for (int i = 0; i < 1000000000; i++) {}
    *newpage = 8;
    // for (int i = 0; i < 1000000000; i++) {}
    // *newpage = 8;
    printd(*newpage);

    register_drivers();
    start_all_drivers();
    find_pci_devices();

    ata_t ata0m;
    init_ata(&ata0m, 0x1F0, 1);
    indentify_ata_device(&ata0m);
    ata_write(&ata0m, "test", 4);
    ata_flush(&ata0m);
    char* read_buffer = (char*)kmalloc(512);
    ata_read(&ata0m, read_buffer);

    for (int i = 0; i < 512; i++) {
        char *text = " \0";
        text[0] = read_buffer[i];
        printf(text);
    }

    asm volatile("int $0"); // test interrupts

    while (1) {}
}
