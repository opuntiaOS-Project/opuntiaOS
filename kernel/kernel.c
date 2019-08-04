#include <display.h>
#include <driverManager.h>
#include <idt.h>
#include <timer.h>
#include <pci.h>



void main() {
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

    __asm__ __volatile__("int $0"); // test interrupts
}