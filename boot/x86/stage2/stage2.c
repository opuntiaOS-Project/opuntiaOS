/**
 * Stage2 is used to load main kernel.
 */

#include "config.h"
#include "drivers/ata.h"
#include "drivers/uart.h"
#include "mem/vm.h"
#include <libboot/abi/drivers.h>
#include <libboot/abi/memory.h>
#include <libboot/crypto/validate.h>
#include <libboot/elf/elf_lite.h>
#include <libboot/fs/ext2_lite.h>
#include <libboot/log/log.h>
#include <libboot/mem/mem.h>
#include <libboot/types.h>

// #define DEBUG_BOOT

static boot_args_t boot_args;

int prepare_boot_disk(drive_desc_t* drive_desc)
{
    init_ata(0x1F0, 1);
    if (indentify_ata_device(drive_desc) == 0) {
        return 0;
    }
    return -1;
}

int prepare_fs(drive_desc_t* drive_desc, fs_desc_t* fs_desc)
{
    if (ext2_lite_init(drive_desc, fs_desc) == 0) {
        return 0;
    }
    return -1;
}

static int validate_kernel(drive_desc_t* drive_desc, fs_desc_t* fs_desc)
{
    log("Validating Kernel...");
    if (!validate_elf(KERNEL_PATH, drive_desc, fs_desc)) {
        log("Can't validate kernel");
        while (1) { }
    }

    if (!validate_elf("/boot/init", drive_desc, fs_desc)) {
        log("Can't validate /boot/init");
        while (1) { }
    }

    return 0;
}

void* bootdesc_ptr;
void load_kernel(drive_desc_t* drive_desc, fs_desc_t* fs_desc, mem_desc_t* mem_desc)
{
    size_t kernel_vaddr = 0;
    size_t kernel_paddr = 0;
    size_t kernel_size = 0;
    int res = elf_load_kernel(drive_desc, fs_desc, KERNEL_PATH, &kernel_vaddr, &kernel_paddr, &kernel_size);
    kernel_size = align_size(kernel_size, VMM_PAGE_SIZE);

    boot_args_t boot_args;
    boot_args.vaddr = kernel_vaddr;
    boot_args.paddr = kernel_paddr;
    boot_args.kernel_size = (kernel_size + align_size(sizeof(boot_args_t), VMM_PAGE_SIZE));
    boot_args.devtree = NULL;
    boot_args.memory_map = (void*)0xA00;
    boot_args.memory_map_size = mem_desc->memory_map_size;
    memcpy(boot_args.init_process, "/boot/init", sizeof("/boot/init"));

    bootdesc_ptr = paddr_to_vaddr(copy_after_kernel(kernel_paddr, &boot_args, sizeof(boot_args), &kernel_size, VMM_PAGE_SIZE), kernel_paddr, kernel_vaddr);
}

void stage2(mem_desc_t* mem_desc)
{
    uart_init();
    log_init(uart_write);

#ifdef DEBUG_BOOT
    log("STAGE2");
#endif
    drive_desc_t drive_desc;
    fs_desc_t fs_desc;

    if (prepare_boot_disk(&drive_desc) != 0) {
#ifdef DEBUG_BOOT
        log("STAGE2");
#endif
        while (1) { }
    }

    if (prepare_fs(&drive_desc, &fs_desc) != 0) {
#ifdef DEBUG_BOOT
        log("STAGE2");
#endif
        while (1) { }
    }

    validate_kernel(&drive_desc, &fs_desc);
    load_kernel(&drive_desc, &fs_desc, mem_desc);
    vm_setup();

    // enabling paging
    asm volatile("mov %cr0, %eax");
    asm volatile("or $0x80000000, %eax");
    asm volatile("mov %eax, %cr0");

    asm volatile("push %0"
                 :
                 : "r"(bootdesc_ptr));
    asm volatile("mov $0xc0000000, %eax");
    asm volatile("call *%eax");
    while (1) { }
}
