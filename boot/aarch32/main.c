/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "drivers/pl181.h"
#include "drivers/uart.h"
#include "target/memmap.h"
#include "vmm/vmm.h"
#include <libboot/devtree/devtree.h>
#include <libboot/elf/elf_lite.h>
#include <libboot/fs/ext2_lite.h>
#include <libboot/log/log.h>
#include <libboot/mem/malloc.h>
#include <libboot/mem/mem.h>

// #define DEBUG_BOOT
#define KERNEL_PATH "/boot/kernel.bin"

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
    size_t alloc_space = (size_t)bootloader_start - dev->paddr;
    malloc_init((void*)dev->paddr, alloc_space);
    return 0;
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

    void* rammap_ptr = paddr_to_vaddr(copy_after_kernel(kernel_paddr, arm_memmap, sizeof(arm_memmap), &kernel_size, VMM_PAGE_SIZE), kernel_paddr, kernel_vaddr);
#ifdef DEBUG_BOOT
    log("copying RAMMAP %x -> %x of %d", arm_memmap, rammap_ptr, sizeof(arm_memmap));
#endif

    boot_desc_t boot_desc;
    boot_desc.vaddr = kernel_vaddr;
    boot_desc.paddr = kernel_paddr;
    boot_desc.kernel_size = (kernel_size + align_size(sizeof(boot_desc_t), VMM_PAGE_SIZE)) / 1024;
    boot_desc.devtree = odt_ptr;
    boot_desc.memory_map = (void*)rammap_ptr;
    boot_desc.memory_map_size = sizeof(arm_memmap) / sizeof(arm_memmap[0]);

    bootdesc_ptr = paddr_to_vaddr(copy_after_kernel(kernel_paddr, &boot_desc, sizeof(boot_desc), &kernel_size, VMM_PAGE_SIZE), kernel_paddr, kernel_vaddr);
#ifdef DEBUG_BOOT
    log("copying BOOTDESC %x -> %x of %d", &boot_desc, bootdesc_ptr, sizeof(boot_desc));
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