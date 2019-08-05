extern "C" {
    #include <display.h>
    #include <driverManager.h>
    #include <idt.h>
    #include <timer.h>
    #include <pci.h>
    #include <types.h>
    #include <ata.h>
}

extern "C" void main() {
    clean_screen();
    {   // interrupts setup
        idt_setup();
        asm volatile("sti");
    }
    {   // drivers setup
        register_drivers();
        start_all_drivers();
        find_pci_devices();
    }
    {
        printf("ATA device");
        ata_t ata0m;
        init_ata(&ata0m, 0x1F0, 1);
        indentify_ata_device(&ata0m);
    }

    asm volatile("int $0"); // test interrupts
}