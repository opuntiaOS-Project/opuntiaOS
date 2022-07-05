/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "elf_lite.h"
#include <libboot/crypto/sha256.h>
#include <libboot/log/log.h>
#include <libboot/mem/alloc.h>
#include <libboot/mem/mem.h>

// #define DEBUG_ELF

int elf_init_ctx(drive_desc_t* drive_desc, fs_desc_t* fs_desc, const char* path, elfctx_t* elfctx)
{
    elfctx->drive_desc = drive_desc;
    elfctx->fs_desc = fs_desc;
    fs_desc->get_inode(drive_desc, path, &elfctx->file_inode);
    fs_desc->read_from_inode(drive_desc, &elfctx->file_inode, (void*)&elfctx->header, 0, sizeof(elfctx->header));

    if (elfctx->header.e_ident[EI_MAG0] != 0x7F || elfctx->header.e_ident[EI_MAG1] != 0x45 || elfctx->header.e_ident[EI_MAG2] != 0x4c || elfctx->header.e_ident[EI_MAG3] != 0x46) {
        return -1;
    }

    return 0;
}

int elf_read_program_header(elfctx_t* elfctx, size_t id, elf_program_header_32_t* program_header)
{
    uintptr_t offset = elfctx->header.e_phoff + id * elfctx->header.e_phentsize;
    return elfctx->fs_desc->read_from_inode(elfctx->drive_desc, &elfctx->file_inode, (void*)program_header, offset, sizeof(*program_header));
}

int elf_read_section_header(elfctx_t* elfctx, uint32_t id, elf_section_header_32_t* section_header)
{
    uintptr_t offset = elfctx->header.e_shoff + id * elfctx->header.e_shentsize;
    return elfctx->fs_desc->read_from_inode(elfctx->drive_desc, &elfctx->file_inode, (void*)section_header, offset, sizeof(*section_header));
}

#ifdef BITS32
int elf_load_kernel(drive_desc_t* drive_desc, fs_desc_t* fs_desc, const char* path, size_t* kernel_vaddr, size_t* kernel_paddr, size_t* kernel_size)
{
    elfctx_t elfctx;
    int err = elf_init_ctx(drive_desc, fs_desc, path, &elfctx);
    if (err) {
        return err;
    }

    size_t vaddr = -1;
    size_t paddr = -1;
    size_t vaddr_end = 0;

    for (uint32_t i = 0; i < elfctx.header.e_phnum; i++) {
        elf_program_header_32_t program_header;
        elf_read_program_header(&elfctx, i, &program_header);

        if (program_header.p_type == PT_LOAD) {
            paddr = min(paddr, program_header.p_paddr);
            vaddr = min(vaddr, program_header.p_vaddr);
            fs_desc->read(drive_desc, path, (void*)program_header.p_paddr, program_header.p_offset, program_header.p_filesz);
        }
    }

    for (uint32_t i = 0; i < elfctx.header.e_shnum; i++) {
        elf_section_header_32_t section_header;
        elf_read_section_header(&elfctx, i, &section_header);

        if ((section_header.sh_flags & SHF_ALLOC) == SHF_ALLOC) {
            vaddr_end = max(vaddr_end, section_header.sh_addr + section_header.sh_size);
        }
    }

    *kernel_vaddr = vaddr;
    *kernel_paddr = paddr;
    *kernel_size = vaddr_end - vaddr;
    return 0;
}
// #endif
#else
size_t elf_get_kernel_size(void* elffile)
{
    elf_header_64_t* header = (elf_header_64_t*)elffile;
    if (header->e_ident[EI_MAG0] != 0x7F || header->e_ident[EI_MAG1] != 0x45 || header->e_ident[EI_MAG2] != 0x4c || header->e_ident[EI_MAG3] != 0x46) {
        return -1;
    }

    size_t vaddr = 0xffffffffffffffff;
    size_t vaddr_end = 0;

    for (size_t i = 0; i < header->e_phnum; i++) {
        uintptr_t offset = header->e_phoff + i * header->e_phentsize;
        elf_program_header_64_t* program_header = (elf_program_header_64_t*)((uintptr_t)elffile + offset);
        if (program_header->p_type == PT_LOAD) {
            vaddr = min(vaddr, program_header->p_vaddr);
        }
    }

    for (size_t i = 0; i < header->e_shnum; i++) {
        uintptr_t offset = header->e_shoff + i * header->e_shentsize;
        elf_section_header_64_t* section_header = (elf_section_header_64_t*)((uintptr_t)elffile + offset);

        if ((section_header->sh_flags & SHF_ALLOC) == SHF_ALLOC) {
            vaddr_end = max(vaddr_end, section_header->sh_addr + section_header->sh_size);
        }
    }

    return vaddr_end - vaddr;
}

int elf_load_kernel(void* elffile, size_t size, uintptr_t* kernel_vaddr, uintptr_t* kernel_paddr)
{
    elf_header_64_t* header = (elf_header_64_t*)elffile;
    if (header->e_ident[EI_MAG0] != 0x7F || header->e_ident[EI_MAG1] != 0x45 || header->e_ident[EI_MAG2] != 0x4c || header->e_ident[EI_MAG3] != 0x46) {
        return -1;
    }

    size_t vaddr = 0xffffffffffffffff;
    for (size_t i = 0; i < header->e_phnum; i++) {
        uintptr_t offset = header->e_phoff + i * header->e_phentsize;
        elf_program_header_64_t* program_header = (elf_program_header_64_t*)((uintptr_t)elffile + offset);
        if (program_header->p_type == PT_LOAD) {
            vaddr = min(vaddr, program_header->p_vaddr);
        }
    }

    uintptr_t paddr = (uintptr_t)palloc(size);
    if (!paddr) {
        return -1;
    }

    for (size_t i = 0; i < header->e_phnum; i++) {
        uintptr_t offset = header->e_phoff + i * header->e_phentsize;
        elf_program_header_64_t* program_header = (elf_program_header_64_t*)((uintptr_t)elffile + offset);
        if (program_header->p_type == PT_LOAD) {
            uintptr_t data_paddr = (program_header->p_vaddr - vaddr) + paddr;
            uintptr_t data_offset = (uintptr_t)elffile + program_header->p_offset;

#ifdef DEBUG_ELF
            log("copying elf %lx %lx %lx", data_paddr, data_offset, program_header->p_filesz);
#endif
            memcpy((void*)data_paddr, (void*)data_offset, program_header->p_filesz);
        }
    }

    *kernel_vaddr = vaddr;
    *kernel_paddr = paddr;
    return 0;
}
#endif