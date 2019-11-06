#include <x86/idt.h>
#include <x86/pci.h>
#include <types.h>
#include <drivers/driver_manager.h>
#include <drivers/ata.h>
#include <drivers/display.h>
#include <drivers/timer.h>
#include <mem/pmm.h>
#include <mem/malloc.h>

#include <cmd/cmd.h>

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

void kpanic(char *t_err_msg);
void ktest();

void kpanic(char *t_err_msg) {
    clean_screen();
    printf("*****\n");
    printf("Kernel Panic\n");
    printf("*****\n");
    printf(t_err_msg);
    printf("\n*****");
    while (1) {}
}

void ktest() {
    {
        uint32_t* kek1 = (uint32_t*)kmalloc(sizeof(uint32_t));
        uint32_t* kek2 = (uint32_t*)kmalloc(sizeof(uint32_t));
        *kek1 = 1;
        *kek2 = 2;
        if (*kek1 == 1 && *kek2 == 2) {
            // printf("\npassed\n");
        } else {
            while (1) {}
        }
    }
    { // page fault new page allocation
        int* newpage = (int *)0x10000000;
        *newpage = 8;
        if (*newpage == 8) {
            // printf("\npassed\n");
        } else {
            while (1) {}
        }
    }


    asm volatile("int $0"); // test interrupts

    printf("\n\nTests passed [ENTER to continue]");

    uint32_t key = KEY_UNKNOWN;
    while (key != KEY_RETURN) {
		key = kbdriver_get_last_key();
    }
    kbdriver_discard_last_key();
}

void load_app(ata_t* ata0m) {
    uint8_t* new_block = pmm_alloc_block();
    vmm_map_page(new_block, 0x60000000);
    uint8_t *app = (uint8_t *)0x60000000;
    printh(app);
    uint8_t* read_buffer = (uint8_t*)kmalloc(512);
    ata_read(ata0m, 60, read_buffer);

    for (int i = 0; i < 256; i+=2) {
        app[i] = read_buffer[i+1];
        app[i+1] = read_buffer[i];
        printh(app[i]); printf(" ");
        printh(app[i+1]); printf(" ");
    }

    printf("\n\nTests passed [ENTER to continue]");

    uint32_t key = KEY_UNKNOWN;
    while (key != KEY_RETURN) {
		key = kbdriver_get_last_key();
    }
    kbdriver_discard_last_key();

    asm volatile("mov $0x60000000, %eax");
    asm volatile("call %eax"); // test interrupts
    uint32_t return_value;
    __asm__("mov %%eax, %%eax" : "=a" (return_value) :);

    printd(return_value);

    printf("\n\nTests passed [ENTER to continue]");

    key = KEY_UNKNOWN;
    while (key != KEY_RETURN) {
		key = kbdriver_get_last_key();
    }
    kbdriver_discard_last_key();
    // while(1) {}
}

void stage3(mem_desc_t *mem_desc) {
    clean_screen();
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
    pmm_init(0x100000, mem_desc->kernel_size, ram_size);
    for (int i = 0; i < mem_desc->memory_map_size; i++) {
        if (memory_map[i].type == 1) {
            pmm_init_region(memory_map[i].startLo, memory_map[i].sizeLo);
        }
    }

    pmm_deinit_mat(); // mat deinit
    pmm_deinit_region(0x0, 0x100000); // kernel stack deinit
    pmm_deinit_region(0x100000, mem_desc->kernel_size * 1024); // kernel deinit

    if (vmm_init()) {
        printf("\nVM Remapped\n");
    } else {
        kpanic("VM Remap error");
    }

    // heap area right after kernel space
    // temp solution will change
    kmalloc_init(0xc0000000 + 0x400000);


    // installing drivers
    pci_install();
    ide_install();
    ata_install();
    kbdriver_install();
    drivers_run();

    // installing devices
    // devices_install();



    // test for searching all storage devices
    device_t cur_dev;
    uint8_t start_s = 0;
    cur_dev.type = DEVICE_STORAGE;
    while (cur_dev.type != DEVICE_BAD_SIGN) {
        cur_dev = get_device(cur_dev.type, start_s);
        if (cur_dev.type != DEVICE_BAD_SIGN) {
            printf("!!! Storage device\n");
        }
        start_s = cur_dev.id + 1;
    }

    ktest();

    // load_app(&ata0m);

    cmd_install();

    while (1) {}
}
