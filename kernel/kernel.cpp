extern "C" {
    #include <display.h>
    #include <driverManager.h>
    #include <idt.h>
    #include <timer.h>
    #include <pci.h>
    #include <types.h>
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

    asm volatile("int $0"); // test interrupts
}