/**
 * Elf lite parese kernel file and load it.
 */

#include "elf_lite.h"
#include "display.h"

elf_header_32_t elf_header;

int elf_load_header(drive_desc_t *drive_desc, fs_desc_t *fs_desc, char *path) {
    int (*read_drive)(drive_desc_t *drive_desc, char *path, uint8_t *buf, uint32_t from, uint32_t len) = fs_desc->read;
    read_drive(drive_desc, path, (uint8_t*)&elf_header, 0, sizeof(elf_header_32_t));

    if (elf_header.e_ident[EI_MAG0] != 0x7F ||
        elf_header.e_ident[EI_MAG1] != 0x45 || 
        elf_header.e_ident[EI_MAG2] != 0x4c ||
        elf_header.e_ident[EI_MAG3] != 0x46) {
        return -1;
    }

    return 0;
}

void elf_read_program_header(drive_desc_t *drive_desc, fs_desc_t *fs_desc, char *path, uint32_t id, elf_program_header_32_t *program_header) {
    int (*read_drive)(drive_desc_t *drive_desc, char *path, uint8_t *buf, uint32_t from, uint32_t len) = fs_desc->read;
    read_drive(drive_desc, path, (uint8_t*)program_header, elf_header.e_phoff + id * elf_header.e_phentsize, sizeof(*program_header));
}

void elf_read_section_header(drive_desc_t *drive_desc, fs_desc_t *fs_desc, char *path, uint32_t id, elf_section_header_32_t *section_header) {
    int (*read_drive)(drive_desc_t *drive_desc, char *path, uint8_t *buf, uint32_t from, uint32_t len) = fs_desc->read;
    read_drive(drive_desc, path, (uint8_t*)section_header, elf_header.e_shoff + id * elf_header.e_shentsize, sizeof(*section_header));
}

int elf_load_kernel(drive_desc_t *drive_desc, fs_desc_t *fs_desc, char *path, uint32_t *kernel_size) {
    int (*read_drive)(drive_desc_t *drive_desc, char *path, uint8_t *buf, uint32_t from, uint32_t len) = fs_desc->read;
    read_drive(drive_desc, path, (uint8_t*)&elf_header, 0, sizeof(elf_header_32_t));

    if (elf_load_header(drive_desc, fs_desc, path) != 0) {
        return -1;
    }
    
    // Reading Kernel
    for (uint32_t i = 0; i < elf_header.e_phnum; i++) {
        elf_program_header_32_t program_header;
        elf_read_program_header(drive_desc, fs_desc, path, i, &program_header);

        // Let's load it
        if (program_header.p_type == PT_LOAD) {
            read_drive(drive_desc, path, (uint8_t*)program_header.p_paddr, program_header.p_offset, program_header.p_filesz);
        }

    }
    
    // Calc Kernel Size
    *kernel_size = 0;
    for (uint32_t i = 0; i < elf_header.e_shnum; i++) {
        elf_section_header_32_t section_header;
        elf_read_section_header(drive_desc, fs_desc, path, i, &section_header);
        
        if (section_header.sh_type == SHT_NOBITS) {
            *kernel_size += section_header.sh_size;
        }
        if (section_header.sh_type == SHT_PROGBITS) {
            *kernel_size += section_header.sh_size;
        }
    }

    return 0;
}