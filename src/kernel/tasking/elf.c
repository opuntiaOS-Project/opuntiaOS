/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <errno.h>
#include <fs/vfs.h>
#include <log.h>
#include <mem/kmalloc.h>
#include <tasking/elf.h>
#include <tasking/tasking.h>

// #define ELF_DEBUG

#define MACHINE_ARCH EM_386
#define USER_STACK_SIZE VMM_PAGE_SIZE

static int _elf_do_copy_to_ram(proc_t* p, file_descriptor_t* fd, elf_program_header_32_t* ph)
{
    uint8_t* buf = kmalloc(ph->p_memsz);
    memset(buf, 0, ph->p_memsz);
    fd->ops->read(fd->dentry, buf, ph->p_offset, ph->p_filesz);
    vmm_copy_to_pdir(p->pdir, buf, ph->p_vaddr, ph->p_memsz);
    kfree(buf);
}

static int _elf_interpret_program_header_entry(proc_t* p, file_descriptor_t* fd)
{
    elf_program_header_32_t ph;
    int err = vfs_read(fd, &ph, sizeof(ph));
    if (err != sizeof(ph)) {
        return err;
    }

#ifdef ELF_DEBUG
    log("Header type %x %x - %x", ph.p_type, ph.p_vaddr, ph.p_memsz);
#endif
    switch (ph.p_type) {
    case PT_LOAD:
        _elf_do_copy_to_ram(p, fd, &ph);
        break;
    default:
        break;
    }
}

static int _elf_interpret_section_header_entry(proc_t* p, file_descriptor_t* fd)
{
    elf_section_header_32_t sh;
    int err = vfs_read(fd, &sh, sizeof(sh));
    if (err != sizeof(sh)) {
        return err;
    }

#ifdef ELF_DEBUG
    log("Section type %x %x - %x", sh.sh_type, sh.sh_addr, sh.sh_size);
#endif
    if (sh.sh_flags & SHF_ALLOC) {
        uint32_t zone_flags = ZONE_READABLE;
        uint32_t zone_type = ZONE_TYPE_NULL;
        
        if (sh.sh_flags & SHF_WRITE) {
            zone_flags |= ZONE_WRITABLE;
        }
        if (sh.sh_flags & SHF_EXECINSTR) {
            zone_flags |= ZONE_EXECUTABLE;
        }
        
        // FIXME: Mapping of zone types is bad.
        switch (sh.sh_type) {
        case SHT_PROGBITS:
            zone_type = ZONE_TYPE_CODE;
            break;
        case SHT_NOBITS:
            zone_type = ZONE_TYPE_DATA;
            break;
        default:
            break;
        }

        if (zone_type) {
            proc_zone_t* zone = proc_extend_zone(p, sh.sh_addr, sh.sh_size);
            if (zone) {
                zone->type = zone_type;
                zone->flags |= zone_flags;
            }
        }
    }
}

static int _elf_alloc_stack(proc_t* p)
{
    proc_zone_t* stack_zone = proc_new_random_zone_backward(p, USER_STACK_SIZE);
    stack_zone->type = ZONE_TYPE_STACK;
    stack_zone->flags |= ZONE_READABLE | ZONE_WRITABLE;
    thread_set_stack(p->main_thread, stack_zone->start + USER_STACK_SIZE, stack_zone->start + USER_STACK_SIZE);
}

static inline int _elf_do_load(proc_t* p, file_descriptor_t* fd, elf_header_32_t* header)
{
    fd->offset = header->e_shoff;
    int sh_num = header->e_shoff;
    for (int i = 0; i < sh_num; i++) {
        _elf_interpret_section_header_entry(p, fd);
    }

    fd->offset = header->e_phoff;
    int ph_num = header->e_phnum;
    for (int i = 0; i < ph_num; i++) {
        _elf_interpret_program_header_entry(p, fd);
    }

    _elf_alloc_stack(p);
    thread_set_eip(p->main_thread, header->e_entry);
    return 0;
}

int elf_load(proc_t* p, file_descriptor_t* fd)
{
    elf_header_32_t header;
    int err = vfs_read(fd, &header, sizeof(header));
    if (err != sizeof(header)) {
        return err;
    }

    char elf_signature[] = { 0x7F, 0x45, 0x4c, 0x46 };
    if (memcmp(header.e_ident, elf_signature, sizeof(elf_signature)) != 0) {
        return -ENOEXEC;
    }

    // FIXME: Currently we support only 32bit execs
    if (header.e_ident[EI_CLASS] != ELF_CLASS_32) {
        return -EBADARCH;
    }

    if (header.e_type != ET_EXEC) {
        return -ENOEXEC;
    }

    if (header.e_machine != MACHINE_ARCH) {
        return -EBADARCH;
    }

    return _elf_do_load(p, fd, &header);
}