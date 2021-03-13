/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libkern/bits/errno.h>
#include <libkern/log.h>
#include <platform/generic/system.h>
#include <platform/generic/tasking/dump_impl.h>
#include <syscalls/handlers.h>
#include <tasking/elf.h>
#include <tasking/proc.h>
#include <tasking/sched.h>
#include <tasking/tasking.h>

#define READ_PER_CYCLE 1024

static int dump_writer(const char* w)
{
    log_not_formatted("%s", w);
    return 0;
}

static int dumper_map_elf_file(proc_t* p, size_t* mapped_at)
{
    // The dumper shares it's vm with the original proc it's dumping,
    // so we create a new zone in the proc and map elf file there.
    file_descriptor_t fd;
    system_disable_interrupts();
    int err = vfs_open(p->proc_file, &fd, O_RDONLY);
    system_enable_interrupts();
    if (err) {
        return err;
    }

    uint32_t elf_file_size = p->proc_file->inode->size;

    system_disable_interrupts();
    proc_zone_t* zone;
    zone = proc_new_random_zone(p, elf_file_size);
    if (!zone) {
        vfs_close(&fd);
        system_enable_interrupts();
        return -ENOMEM;
    }
    zone->flags |= ZONE_READABLE;
    system_enable_interrupts();

    // Use kernel hack and read straigth to our buffer. It's implemented in parts,
    // not to take much uninterruptable time reading our symtable.
    char* copy_to = (char*)zone->start;
    vmm_prepare_active_pdir_for_copying_at(zone->start, zone->len);
    for (uint32_t read = 0; read < elf_file_size; read += READ_PER_CYCLE) {
        system_disable_interrupts();
        fd.ops->read(fd.dentry, copy_to, read, READ_PER_CYCLE);
        system_enable_interrupts();
        copy_to += READ_PER_CYCLE;
    }

    *mapped_at = zone->start;
    system_disable_interrupts();
    vfs_close(&fd);
    system_enable_interrupts();
    return 0;
}

static void dumper(proc_t* p)
{
    int err = 0;
    dump_data_t dump_data = { 0 };
    dump_data.p = p;

    size_t mapped_at = 0;
    err = dumper_map_elf_file(p, &mapped_at);
    if (err) {
        goto exit;
    }

    void* mapped_at_ptr = (void*)mapped_at;
    err = elf_check_header(mapped_at_ptr);
    if (err) {
        goto exit;
    }

    err = elf_find_symtab_unchecked(mapped_at_ptr, &dump_data.syms, &dump_data.symsn, &dump_data.strs);
    if (err || !dump_data.syms || !dump_data.strs) {
        goto exit;
    }

    dump_data.entry_point = ((elf_header_32_t*)mapped_at_ptr)->e_entry;
    dump_data.writer = dump_writer;
    dump_data.sym_resolver = elf_find_function_in_symtab;
    err = dump_impl(&dump_data);

exit:
    proc_die(p);
    ksys1(SYSEXIT, err);
}

void dump_and_kill(proc_t* p)
{
    blocker_t blocker;
    blocker.reason = BLOCKER_DUMPING;
    blocker.should_unblock = NULL;
    blocker.should_unblock_for_signal = false;
    proc_block_all_threads(p, &blocker);
    proc_t* dumper_p = tasking_create_kernel_thread(dumper, p);

    // Kthread does NOT clean it's pdir, so we can share the pdir of
    // the blocked proc to read it's content.
    dumper_p->pdir = p->pdir;

    resched();
}