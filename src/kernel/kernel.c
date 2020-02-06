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

void stage3(mem_desc_t *mem_desc) {
    clean_screen();
    gdt_setup();
    idt_setup();
    // init_timer();

    pmm_setup(mem_desc);

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