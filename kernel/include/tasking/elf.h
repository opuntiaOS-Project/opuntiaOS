/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_TASKING_ELF_H
#define _KERNEL_TASKING_ELF_H

#define ELF_CLASS_32 1
#define ELF_CLASS_64 2

#define ELF_DATA2_LSB 1
#define ELF_DATA2_MSB 2

enum E_IDENT_FIELDS {
    EI_MAG0,
    EI_MAG1,
    EI_MAG2,
    EI_MAG3,
    EI_CLASS,
    EI_DATA,
    EI_VERSION,
    EI_OSABI,
    EI_ABIVERSION,
};

enum E_TYPE_FIELDS {
    ET_NONE,
    ET_REL,
    ET_EXEC,
    ET_DYN,
    ET_CORE,
};

enum E_MACHINE_FIELDS {
    EM_NONE = 0x0,
    EM_386 = 0x03,
    EM_ARM = 0x28,
    EM_AMD64 = 0x32,
};

typedef struct {
    uint8_t e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} elf_header_32_t;

enum P_TYPE_FIELDS {
    PT_NULL,
    PT_LOAD,
    PT_DYNAMIC,
    PT_INTERP,
    PT_NOTE,
    PT_SHLIB,
    PT_PHDR,
    PT_TLS,
    PT_LOOS = 0x60000000,
    PT_HIOS = 0x6FFFFFFF,
    PT_LOPROC = 0x70000000,
    PT_HIPROC = 0x7FFFFFFF,
};

typedef struct {
    uint32_t p_type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_flags;
    uint32_t p_align;
} elf_program_header_32_t;

enum SH_TYPE_FIELDS {
    SHT_NULL,
    SHT_PROGBITS,
    SHT_SYMTAB,
    SHT_STRTAB,
    SHT_RELA,
    SHT_HASH,
    SHT_DYNAMIC,
    SHT_NOTE,
    SHT_NOBITS,
    SHT_REL,
    SHT_SHLIB,
    SHT_DYNSYM,
    SHT_INIT_ARRAY,
    SHT_FINI_ARRAY,
    SHT_PREINIT_ARRAY,
    SHT_GROUP,
    SHT_SYMTAB_SHNDX,

    SHT_LOOS = 1610612736,
    SHT_HIOS = 1879048191,
    SHT_LOPROC = 1879048192,
    SHT_HIPROC = 2147483647,
    SHT_LOUSER = 2147483648,
    SHT_HIUSER = 4294967295,
};

enum SH_FLAGS_FIELDS {
    SHF_WRITE = 0x1,
    SHF_ALLOC = 0x2,
    SHF_EXECINSTR = 0x4,
    SHF_MERGE = 0x10,
    SHF_STRINGS = 0x20,

    SHF_INFO_LINK = 0x40,
    SHF_LINK_ORDER = 0x80,
    SHF_OS_NONCONFORMING = 0x100,
    SHF_GROUP = 0x200,
    SHF_TLS = 0x400,
    SHF_COMPRESSED = 0x800,
    SHF_MASKOS = 0x0ff00000,
    SHF_MASKPROC = 0xf0000000
};

typedef struct {
    uint32_t sh_name;
    uint32_t sh_type;
    uint32_t sh_flags;
    uint32_t sh_addr;
    uint32_t sh_offset;
    uint32_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint32_t sh_addralign;
    uint32_t sh_entsize;
} elf_section_header_32_t;

typedef struct {
    uint32_t st_name; /* name - index into string table */
    uint32_t st_value; /* symbol value */
    uint32_t st_size; /* symbol size */
    unsigned char st_info; /* type and binding */
    unsigned char st_other; /* 0 - no defined meaning */
    uint16_t st_shndx; /* section header index */
} elf_sym_32_t;

enum ST_BINDING_FIELDS {
    STB_LOCAL = 0,
    STB_GLOBAL = 1,
    STB_WEAK = 2,
    STB_NUM = 3,
    STB_LOPROC = 13,
    STB_HIPROC = 15
};

enum ST_TYPE_FIELDS {
    STT_NOTYPE = 0,
    STT_OBJECT = 1,
    STT_FUNC = 2,
    STT_SECTION = 3,
    STT_FILE = 4,
    STT_TLS = 6,
    STT_LOPROC = 13,
    STT_HIPROC = 15
};

struct proc;
struct file_descriptor;

int elf_check_header(elf_header_32_t* header);
int elf_load(struct proc* p, struct file_descriptor* fd);
int elf_find_symtab_unchecked(void* mapped_data, void** symtab, size_t* symtab_entries, char** strtab);
ssize_t elf_find_function_in_symtab(void* symtab, size_t syms_n, uint32_t ip);

#endif // _KERNEL_TASKING_ELF_H