#include <display.h>
#include <driverManager.h>
#include <idt.h>
#include <timer.h>
#include <pci.h>



void main() {
    clean_screen();
    idt_setup();
    asm volatile("sti");

    init_drivers();
    find_pci_devices();

    __asm__ __volatile__("int $0");
    // for (int i = 0; i < 80 * 25; i++) {
    //     print_char((char)('0' + (i/80)%10), WHITE_ON_BLACK, -1, -1);
    // }
    // print_string("Starting\nKernel\nWith NEWLINE Support\n", BLACK_ON_WHITE, 0, 0);
}