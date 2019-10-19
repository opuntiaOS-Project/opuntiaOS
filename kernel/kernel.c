#include <display.h>
#include <driverManager.h>
#include <idt.h>
#include <timer.h>
#include <pci.h>
#include <types.h>
#include <ata.h>


void main() {
    clean_screen();
    idt_setup();
    asm volatile("sti");
    register_drivers();
    start_all_drivers();
    find_pci_devices();
    printf("ATA device");
    ata_t ata0m;
    init_ata(&ata0m, 0x1F0, 1);
    indentify_ata_device(&ata0m);
    
    //asm volatile("int $118");

    ata_write(&ata0m, "AABBCCDD", 8);
    ata_flush(&ata0m);
   

    for (int i = 0; i < 50; i++) {
        ata_read(&ata0m);
        //ata_flush(&ata0m);
    }

    // ata_read(&ata0m);
    
    
    asm volatile("int $0"); // test interrupts

    while (1) {}
}