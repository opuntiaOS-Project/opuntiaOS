/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _BOOT_LIBBOOT_ABI_MULTIBOOT_H
#define _BOOT_LIBBOOT_ABI_MULTIBOOT_H

#include <libboot/types.h>

#define MULTIBOOT_INFO_MEMORY (0x1)
#define MULTIBOOT_INFO_BOOTDEV (0x2)
#define MULTIBOOT_INFO_CMDLINE (0x4)
#define MULTIBOOT_INFO_MODS (0x8)
#define MULTIBOOT_INFO_AOUT_SYMS (0x10)
#define MULTIBOOT_INFO_ELF_SHDR (0x20)
#define MULTIBOOT_INFO_MEM_MAP (0x40)
#define MULTIBOOT_INFO_DRIVE_INFO (0x80)
#define MULTIBOOT_INFO_CONFIG_TABLE (0x100)
#define MULTIBOOT_INFO_BOOT_LOADER_NAME (0x200)
#define MULTIBOOT_INFO_APM_TABLE (0x400)
#define MULTIBOOT_INFO_VBE_INFO (0x800)
#define MULTIBOOT_INFO_FRAMEBUFFER_INFO (0x1000)

struct multiboot_module_entry {
    uint32_t start;
    uint32_t end;
    uint32_t string_addr;
    uint32_t reserved;
};
typedef struct multiboot_module_entry multiboot_module_entry_t;

struct multiboot_aout_symbol_table {
    uint32_t tabsize;
    uint32_t strsize;
    uint32_t addr;
    uint32_t reserved;
};
typedef struct multiboot_aout_symbol_table multiboot_aout_symbol_table_t;

struct multiboot_elf_section_header_table {
    uint32_t num;
    uint32_t size;
    uint32_t addr;
    uint32_t shndx;
};
typedef struct multiboot_elf_section_header_table multiboot_elf_section_header_table_t;

#define MULTIBOOT_MEMORY_AVAILABLE 1
#define MULTIBOOT_MEMORY_RESERVED 2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE 3
#define MULTIBOOT_MEMORY_NVS 4
#define MULTIBOOT_MEMORY_BADRAM 5
struct PACKED multiboot_mmap_entry {
    uint32_t size;
    uint64_t addr;
    uint64_t len;
    uint32_t type;
};
typedef struct multiboot_mmap_entry multiboot_memory_map_t;

#define MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED 0
#define MULTIBOOT_FRAMEBUFFER_TYPE_RGB 1
#define MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT 2
struct multiboot_info {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    union {
        multiboot_aout_symbol_table_t aout_sym;
        multiboot_elf_section_header_table_t elf_sec;
    } syms;
    uint32_t mmap_length;
    uint32_t mmap_addr;
    uint32_t drives_length;
    uint32_t drives_addr;
    uint32_t config_table;
    uint32_t boot_loader_name;
    uint32_t apm_table;
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;
    uint8_t framebuffer_type;
    union {
        struct
        {
            uint32_t framebuffer_palette_addr;
            uint16_t framebuffer_palette_num_colors;
        };
        struct
        {
            uint8_t framebuffer_red_field_position;
            uint8_t framebuffer_red_mask_size;
            uint8_t framebuffer_green_field_position;
            uint8_t framebuffer_green_mask_size;
            uint8_t framebuffer_blue_field_position;
            uint8_t framebuffer_blue_mask_size;
        };
    };
};
typedef struct multiboot_info multiboot_info_t;

#endif // _BOOT_LIBBOOT_ABI_MULTIBOOT_H