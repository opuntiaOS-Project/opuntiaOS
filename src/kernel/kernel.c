#include <x86/idt.h>
#include <x86/pci.h>
#include <types.h>
#include <drivers/driverManager.h>
#include <drivers/ata.h>
#include <drivers/display.h>
#include <drivers/timer.h>

#include <qemulog.h>

void main() {
    log("Qemu debug in work\n");
    clean_screen();
    idt_setup();
    asm volatile("sti");
    register_drivers();
    start_all_drivers();
    find_pci_devices();
    
    ata_t ata0m;
    init_ata(&ata0m, 0x1F0, 1);
    indentify_ata_device(&ata0m);
    ata_write(&ata0m, "test", 4);
    ata_flush(&ata0m);
    ata_read(&ata0m);
    
    asm volatile("int $0"); // test interrupts
    while (1) {}
}