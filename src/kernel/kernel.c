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

    // test.s program
    char *ram = 0x100000;
    ram[0] = 0x66;
    ram[1] = 0xb8;
    ram[2] = 0x03;
    ram[3] = 0x00;
    ram[4] = 0x00;
    ram[5] = 0x00;
    ram[6] = 0x66;
    ram[7] = 0xbb;
    ram[8] = 0x06;
    ram[9] = 0x00;
    ram[10] = 0x00;
    ram[11] = 0x00;
    ram[12] = 0xcd;
    ram[13] = 0x00;
    ram[14] = 0xeb;
    ram[15] = 0xfe;
    
    asm volatile("mov $0x100000, %eax");
    asm volatile("mov %eax, %ebp");
    asm volatile("jmp %eax");
    
    
    asm volatile("int $0"); // test interrupts
    while (1) {}
}