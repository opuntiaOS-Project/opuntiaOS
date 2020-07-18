#include <x86/idt.h>
#include <x86/gdt.h>
#include <x86/pci.h>

#include <types.h>

#include <drivers/driver_manager.h>
#include <drivers/ata.h>
#include <drivers/display.h>
#include <drivers/timer.h>
#include <drivers/ide.h>

#include <mem/pmm.h>
#include <mem/kmalloc.h>

#include <fs/ext2/ext2.h>
#include <fs/devfs/devfs.h>
#include <fs/procfs/procfs.h>
#include <fs/vfs.h>

#include <cmd/cmd.h>
#include <cmd/system_commands.h>

#include <tasking/sched.h>

#include <qemulog.h>
#include <utils/kernel_self_test.h>

void stage3(mem_desc_t *mem_desc) {
    clean_screen();
    gdt_setup();
    idt_setup();
    init_timer();

    // mem setup
    pmm_setup(mem_desc);
    vmm_setup();
    
    // kernel self test
    kernel_self_test(true);

    // installing drivers
    driver_manager_init();
    pci_install();
    ide_install();
    ata_install();
    kbdriver_install();
    vfs_install();
    ext2_install();
    procfs_install();
    devfs_install();
    drivers_run();

    tasking_init();
    scheduler_init();

    syscmd_init();
    cmd_install();

    while (1) { }
}