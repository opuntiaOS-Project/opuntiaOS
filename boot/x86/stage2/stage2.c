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
#include <libboot/mem/alloc.h>
#include <libboot/mem/mem.h>
#include <libboot/types.h>

// #define DEBUG_BOOT
#define KERNEL_PATH "/boot/kernel.bin"
#define LAUNCH_SERVER_PATH "/System/launch_server"

static boot_args_t boot_args;
static size_t kernel_vaddr = 0;
static size_t kernel_paddr = 0;
static size_t kernel_size = 0;

static void alloc_init(x86_mem_desc_t* mem_desc)
{
    // TODO: We use manual layouting in this bootloader. Thus
    // all addresses are hardcoded, no need to use allocator at all.
}

static memory_boot_desc_t memory_boot_desc_init(x86_mem_desc_t* mem_desc)
{
    char dummy_data = 0x0;
    size_t ram_last_addr = 0x0;
    size_t next_id = 0;
    memory_layout_t* mem_layout_paddr = copy_after_kernel(kernel_paddr, &dummy_data, sizeof(dummy_data), &kernel_size, VMM_PAGE_SIZE);
    x86_memory_map_t* memory_map = (x86_memory_map_t*)0xA00;

    for (int i = 0; i < mem_desc->memory_map_size; i++) {
        if (memory_map[i].type == 0x2) {
            // The region is marked as reserved, removing it.
            mem_layout_paddr[next_id].base = memory_map[i].startLo;
            mem_layout_paddr[next_id].size = memory_map[i].sizeLo;
            mem_layout_paddr[next_id].flags = 0;
            next_id++;
        } else {
            ram_last_addr = max(ram_last_addr, memory_map[i].startLo + memory_map[i].sizeLo);
        }
    }
    mem_layout_paddr[next_id].flags = MEMORY_LAYOUT_FLAG_TERMINATE;
    memory_layout_t* mem_layout_vaddr = paddr_to_vaddr(mem_layout_paddr, kernel_paddr, kernel_vaddr);

    memory_boot_desc_t res;
    res.ram_base = 0x0;
    res.ram_size = ram_last_addr;
    res.reserved_areas = mem_layout_vaddr;
    return res;
}

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

    if (!validate_elf(LAUNCH_SERVER_PATH, drive_desc, fs_desc)) {
        log("Can't validate launch_server");
        while (1) { }
    }

    return 0;
}

void* bootdesc_ptr;
void load_kernel(drive_desc_t* drive_desc, fs_desc_t* fs_desc, x86_mem_desc_t* mem_desc)
{
    int res = elf_load_kernel(drive_desc, fs_desc, KERNEL_PATH, &kernel_vaddr, &kernel_paddr, &kernel_size);
    kernel_size = align_size(kernel_size, VMM_PAGE_SIZE);

    size_t kernel_data_size = kernel_size + align_size(sizeof(boot_args_t), VMM_PAGE_SIZE) + VMM_PAGE_SIZE;

    boot_args_t boot_args;
    boot_args.vaddr = kernel_vaddr;
    boot_args.paddr = kernel_paddr;
    boot_args.kernel_data_size = kernel_data_size;
    boot_args.devtree = NULL;
    boot_args.mem_boot_desc = memory_boot_desc_init(mem_desc);
    memcpy(boot_args.init_process, LAUNCH_SERVER_PATH, sizeof(LAUNCH_SERVER_PATH));

    bootdesc_ptr = paddr_to_vaddr(copy_after_kernel(kernel_paddr, &boot_args, sizeof(boot_args), &kernel_size, VMM_PAGE_SIZE), kernel_paddr, kernel_vaddr);
}

void stage2(x86_mem_desc_t* mem_desc)
{
    uart_init();
    log_init(uart_write);
    alloc_init(mem_desc);

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
