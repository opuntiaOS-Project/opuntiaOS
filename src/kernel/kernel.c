#include <x86/idt.h>
#include <x86/gdt.h>
#include <x86/pci.h>
#include <types.h>
#include <drivers/driver_manager.h>
#include <drivers/ata.h>
#include <drivers/display.h>
#include <drivers/timer.h>
#include <mem/pmm.h>
#include <mem/malloc.h>

#include <fs/fat16/fat16.h>
#include <fs/vfs.h>

#include <cmd/cmd.h>
#include <cmd/system_commands.h>

#include <qemulog.h>
#include <utils/kernel_self_test.h>

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
    clean_screen();
    gdt_setup();
    idt_setup();
    asm volatile("sti");
    // init_timer();

    printf("Kernel size: "); printd(mem_desc->kernel_size); printf("KB\n");
    uint32_t ram_size = 0;
    memory_map_t *memory_map = (memory_map_t *)MEMORY_MAP_REGION;
    for (int i = 0; i < mem_desc->memory_map_size; i++) {
        if (memory_map[i].type == 1) {
            ram_size = memory_map[i].startLo + memory_map[i].sizeLo;
        }
    }
    printf("Ram size: "); printh(ram_size);
    pmm_init(0xc0000000, mem_desc->kernel_size, ram_size);
    for (int i = 0; i < mem_desc->memory_map_size; i++) {
        if (memory_map[i].type == 1) {
            pmm_init_region(memory_map[i].startLo, memory_map[i].sizeLo);
        }
    }

    pmm_deinit_mat(); // mat deinit
    pmm_deinit_region(0x0, 0x100000); // kernel stack deinit
    pmm_deinit_region(0x100000, mem_desc->kernel_size * 1024); // stage2 deinit

    if (vmm_init()) {
        printf("\nVM Remapped\n");
    } else {
        kpanic("VM Remap error");
    }

    // heap area right after kernel space
    // temp solution will change
    kmalloc_init(0xc0000000 + 0x400000);
    
    // kernel self test
    kernel_self_test(true);

    // installing drivers
    driver_manager_init();
    pci_install();
    ide_install();
    ata_install();
    kbdriver_install();
    vfs_install();
    fat16_install();
    drivers_run();

    syscmd_init();
    cmd_install();

    while (1) {}
}