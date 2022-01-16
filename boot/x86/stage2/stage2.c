/**
 * Stage2 is used to load main kernel.
 */

#include "config.h"
#include "drivers/ata.h"
#include "drivers/uart.h"
#include "mem/vm.h"
#include <libboot/abi/drivers.h>
#include <libboot/abi/memory.h>
#include <libboot/elf/elf_lite.h>
#include <libboot/fs/ext2_lite.h>
#include <libboot/log/log.h>
#include <libboot/string/string.h>
#include <libboot/types.h>

// #define DEBUG_BOOT

static boot_desc_t boot_desc;

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

static size_t size_to_page_boundary(size_t size)
{
    if (size % VMM_PAGE_SIZE) {
        size += VMM_PAGE_SIZE - (size % VMM_PAGE_SIZE);
    }
    return size;
}

void* copy_after_kernel(size_t kbase, void* from, size_t size, size_t* kernel_size)
{
    void* pp = (void*)(kbase + *kernel_size);
    memcpy(pp, from, size);
    *kernel_size += size_to_page_boundary(size);
    return pp;
}

void* paddr_to_vaddr(void* ptr, size_t pbase, size_t vbase)
{
    return (void*)((size_t)ptr - pbase + vbase);
}

void* bootdesc_ptr;
void load_kernel(drive_desc_t* drive_desc, fs_desc_t* fs_desc, mem_desc_t* mem_desc)
{
    size_t kernel_vaddr = 0;
    size_t kernel_paddr = 0;
    size_t kernel_size = 0;
    int res = elf_load_kernel(drive_desc, fs_desc, KERNEL_PATH, &kernel_vaddr, &kernel_paddr, &kernel_size);

    boot_desc_t boot_desc;
    boot_desc.vaddr = kernel_vaddr;
    boot_desc.paddr = kernel_paddr;
    boot_desc.kernel_size = (kernel_size + size_to_page_boundary(sizeof(boot_desc_t))) / 1024;
    boot_desc.devtree = NULL;
    boot_desc.memory_map = (void*)0xA00;
    boot_desc.memory_map_size = mem_desc->memory_map_size;

    bootdesc_ptr = paddr_to_vaddr(copy_after_kernel(kernel_paddr, &boot_desc, sizeof(boot_desc), &kernel_size), kernel_paddr, kernel_vaddr);
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

    load_kernel(&drive_desc, &fs_desc, mem_desc);
    vm_setup();

    // enabling paging
    asm volatile("mov %cr0, %eax");
    asm volatile("or $0x80000000, %eax");
    asm volatile("mov %eax, %cr0");

    asm volatile("add $0xffc00000, %ebp");
    asm volatile("add $0xffc00000, %esp");

    asm volatile("push %0"
                 :
                 : "r"(bootdesc_ptr));
    asm volatile("mov $0xc0000000, %eax");
    asm volatile("call *%eax");
    while (1) { }
}
