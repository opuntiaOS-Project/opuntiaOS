/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "drivers/pl181.h"
#include "drivers/uart.h"
#include "vmm/vmm.h"
#include <libboot/crypto/sha256.h>
#include <libboot/crypto/signature.h>
#include <libboot/crypto/uint2048.h>
#include <libboot/crypto/validate.h>
#include <libboot/devtree/devtree.h>
#include <libboot/elf/elf_lite.h>
#include <libboot/fs/ext2_lite.h>
#include <libboot/log/log.h>
#include <libboot/mem/alloc.h>
#include <libboot/mem/mem.h>

// #define DEBUG_BOOT
#define KERNEL_PATH "/boot/kernel.bin"
#define LAUNCH_SERVER_PATH "/System/launch_server"

extern void jump_to_kernel(void*);
extern uint32_t _odt_phys[];
extern uint32_t _odt_phys_end[];

static void* bootdesc_ptr;
static size_t kernel_vaddr = 0;
static size_t kernel_paddr = 0;
static size_t kernel_size = 0;
static int sync = 0;

static int alloc_init()
{
    devtree_entry_t* dev = devtree_find_device("ram");
    if (!dev) {
        log("Can't find RAM in devtree");
        while (1) { };
    }

    extern int bootloader_start[];
    size_t alloc_space = (size_t)bootloader_start - dev->region_base;
    malloc_init((void*)(uint32_t)dev->region_base, alloc_space);
    return 0;
}

static memory_boot_desc_t memory_boot_desc_init()
{
    devtree_entry_t* dev = devtree_find_device("ram");
    if (!dev) {
        log("Can't find RAM in devtree");
        while (1) { };
    }

    char dummy_data = 0x0;
    memory_boot_desc_t res;
    memory_layout_t* mem_layout_paddr = copy_after_kernel(kernel_paddr, &dummy_data, sizeof(dummy_data), &kernel_size, VMM_PAGE_SIZE);

    // Init of trailing element.
    mem_layout_paddr[0].flags = MEMORY_LAYOUT_FLAG_TERMINATE;
    memory_layout_t* mem_layout_vaddr = paddr_to_vaddr(mem_layout_paddr, kernel_paddr, kernel_vaddr);
#ifdef DEBUG_BOOT
    log("initing MEMLAYOUT %lx of %d elems", mem_layout_vaddr, memory_layout_size);
#endif

    res.ram_base = dev->region_base;
    res.ram_size = dev->region_size;
    res.reserved_areas = mem_layout_vaddr;
    return res;
}

static int prepare_boot_disk(drive_desc_t* drive_desc)
{
    pl181_init(drive_desc);
    return -1;
}

static int prepare_fs(drive_desc_t* drive_desc, fs_desc_t* fs_desc)
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

static void load_kernel(drive_desc_t* drive_desc, fs_desc_t* fs_desc)
{
    int res = elf_load_kernel(drive_desc, fs_desc, KERNEL_PATH, &kernel_vaddr, &kernel_paddr, &kernel_size);
    kernel_size = align_size(kernel_size, VMM_PAGE_SIZE);
#ifdef DEBUG_BOOT
    log("kernel %x %x %x", kernel_vaddr, kernel_paddr, kernel_size);
#endif

    void* odt_ptr = paddr_to_vaddr(copy_after_kernel(kernel_paddr, devtree_start(), devtree_size(), &kernel_size, VMM_PAGE_SIZE), kernel_paddr, kernel_vaddr);
#ifdef DEBUG_BOOT
    log("copying ODT %x -> %x of %d", devtree_start(), odt_ptr, devtree_size());
#endif

    size_t kernel_data_size = kernel_size + align_size(sizeof(boot_args_t), VMM_PAGE_SIZE) + VMM_PAGE_SIZE;

    boot_args_t boot_args;
    boot_args.vaddr = kernel_vaddr;
    boot_args.paddr = kernel_paddr;
    boot_args.kernel_data_size = kernel_data_size;
    boot_args.devtree = odt_ptr;
    boot_args.mem_boot_desc = memory_boot_desc_init();
    memcpy(boot_args.init_process, LAUNCH_SERVER_PATH, sizeof(LAUNCH_SERVER_PATH));

    bootdesc_ptr = paddr_to_vaddr(copy_after_kernel(kernel_paddr, &boot_args, sizeof(boot_args), &kernel_size, VMM_PAGE_SIZE), kernel_paddr, kernel_vaddr);
#ifdef DEBUG_BOOT
    log("copying BOOTDESC %x -> %x of %d", &boot_args, bootdesc_ptr, sizeof(boot_args));
#endif
}

void load_boot_cpu()
{
    devtree_init((void*)_odt_phys, (uint32_t)_odt_phys_end - (uint32_t)_odt_phys);
    uart_init();
    log_init(uart_write);
    alloc_init();

    drive_desc_t drive_desc;
    fs_desc_t fs_desc;
    prepare_boot_disk(&drive_desc);
    prepare_fs(&drive_desc, &fs_desc);
    validate_kernel(&drive_desc, &fs_desc);
    load_kernel(&drive_desc, &fs_desc);
    vm_setup(kernel_vaddr, kernel_paddr, kernel_size);

    __atomic_store_n(&sync, 1, __ATOMIC_RELEASE);
    jump_to_kernel(bootdesc_ptr);
}

void load_secondary_cpu()
{
    while (__atomic_load_n(&sync, __ATOMIC_ACQUIRE) == 0) {
        continue;
    }
    vm_setup_secondary_cpu();
    jump_to_kernel(bootdesc_ptr);
}