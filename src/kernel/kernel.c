#include <x86/idt.h>
#include <x86/pci.h>
#include <types.h>
#include <drivers/driverManager.h>
#include <drivers/ata.h>
#include <drivers/display.h>
#include <drivers/timer.h>

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

void main(uint16_t *memory_map_size) {
    log("Qemu debug in work\n");
    clean_screen();
    memory_map_t *memory_map = (memory_map_t *)MEMORY_MAP_REGION;
    for (int i = 0; i < *memory_map_size; i++) {
        printh(memory_map[i].startLo); printf(" ");
        printd(memory_map[i].sizeLo); printf(" ");
        printh(memory_map[i].type); printf(" ");
        printf("\n");
    }
    idt_setup();
    asm volatile("sti");
    register_drivers();
    start_all_drivers();
    // find_pci_devices();
    
    ata_t ata0m;
    init_ata(&ata0m, 0x1F0, 1);
    indentify_ata_device(&ata0m);
    ata_write(&ata0m, "test", 4);
    ata_flush(&ata0m);
    ata_read(&ata0m);

    asm volatile("int $0"); // test interrupts
    while (1) {}
}