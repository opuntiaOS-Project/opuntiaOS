#include <x86/common.h>
#include <x86/gdt.h>
#include <x86/idt.h>
#include <x86/pci.h>

#include <types.h>

#include <drivers/ata.h>
#include <drivers/display.h>
#include <drivers/driver_manager.h>
#include <drivers/ide.h>
#include <drivers/mouse.h>
#include <drivers/pit.h>

#include <mem/kmalloc.h>
#include <mem/pmm.h>

#include <fs/devfs/devfs.h>
#include <fs/ext2/ext2.h>
#include <fs/procfs/procfs.h>
#include <fs/vfs.h>

#include <io/tty/tty.h>

#include <time/time_manager.h>

#include <cmd/cmd.h>
#include <cmd/system_commands.h>

#include <tasking/sched.h>

#include <qemulog.h>
#include <utils/kernel_self_test.h>

#include <sys_handler.h>

/* If we stay our anythread alone it can't get keyboard input,
   so we will switch to idle_thread to fix it. */
void idle_thread()
{
    while (1);
}

// FIXME
void launching()
{
    tasking_create_kernel_thread(idle_thread);
    tasking_create_kernel_thread(dentry_flusher);
    tasking_start_init_proc();
    ksys1(SYSEXIT, 0);
}

void stage3(mem_desc_t* mem_desc)
{
    clean_screen();
    gdt_setup();
    idt_setup();
    pit_setup();

    // mem setup
    pmm_setup(mem_desc);
    vmm_setup();

    // kernel self test
    kernel_self_test(true);

    timeman_setup();

    // installing drivers
    driver_manager_init();
    pci_install();
    ide_install();
    ata_install();
    kbdriver_install();
    mouse_install();
    vfs_install();
    ext2_install();
    procfs_install();
    devfs_install();
    drivers_run();

    devfs_mount();

    tasking_init();
    scheduler_init();

    syscmd_init();
    cli();
    clean_screen();
    tasking_create_kernel_thread(launching);
    resched(); /* Starting a scheduler */

    while (1);
}