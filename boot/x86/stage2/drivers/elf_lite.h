#ifndef STAGE2_ELF_LITE
#define STAGE2_ELF_LITE

#include "../types.h"
#include "drive_desc.h"
#include "fs_desc.h"

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
    EM_NONE,
    EM_386 = 3,
    EM_PPC = 20,
    EM_PPC64 = 21,
    EM_X86_64 = 62,
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
    uint16_t p_memsz;
    uint16_t p_flags;
    uint16_t p_align;
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

int elf_load_header(drive_desc_t* drive_desc, fs_desc_t* fs_desc, char* path);
int elf_load_kernel(drive_desc_t* drive_desc, fs_desc_t* fs_desc, char* path, uint32_t* kernel_size);

#endif // STAGE2_ELF_LITE