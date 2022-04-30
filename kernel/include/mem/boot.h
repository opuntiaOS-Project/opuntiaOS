#ifndef _KERNEL_MEM_BOOT_H
#define _KERNEL_MEM_BOOT_H

#include <libkern/types.h>

struct memory_map {
    uint32_t startLo;
    uint32_t startHi;
    uint32_t sizeLo;
    uint32_t sizeHi;
    uint32_t type;
    uint32_t acpi_3_0;
};
typedef struct memory_map memory_map_t;

struct fb_boot_desc {
    uintptr_t vaddr;
    uintptr_t paddr;
    size_t width;
    size_t height;
    size_t pixels_per_row;
};
typedef struct fb_boot_desc fb_boot_desc_t;

struct boot_args {
    size_t paddr;
    size_t vaddr;
    void* memory_map;
    size_t memory_map_size;
    size_t kernel_size;
    void* devtree;
    fb_boot_desc_t fb_boot_desc;
    char cmd_args[32];
    char init_process[32];
};
typedef struct boot_args boot_args_t;

#endif // _KERNEL_MEM_BOOT_H