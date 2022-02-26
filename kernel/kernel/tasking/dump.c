/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
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

kmemzone_t kernel_file_mapping_zone;
dump_data_t kernel_dump_data;

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

    size_t elf_file_size = p->proc_file->inode->size;

    system_disable_interrupts();
    memzone_t* zone;
    zone = memzone_new_random(p->address_space, elf_file_size);
    if (!zone) {
        vfs_close(&fd);
        system_enable_interrupts();
        return -ENOMEM;
    }
    zone->mmu_flags |= MMU_FLAG_PERM_READ;
    system_enable_interrupts();

    // Use kernel hack and read straigth to our buffer. It's implemented in parts,
    // not to take much uninterruptable time reading our symtable.
    uint8_t* copy_to = (uint8_t*)zone->start;
    vmm_ensure_writing_to_active_address_space(zone->start, zone->len);
    for (size_t read = 0; read < elf_file_size; read += READ_PER_CYCLE) {
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
    ksys1(SYS_EXIT, err);
}

void dump_and_kill(proc_t* p)
{
    blocker_t blocker;
    blocker.reason = BLOCKER_DUMPING;
    blocker.should_unblock = NULL;
    blocker.should_unblock_for_signal = false;
    proc_block_all_threads(p, &blocker);
    proc_t* dumper_p = tasking_run_kernel_thread(dumper, p);

    // Hack: kthreads do NOT clean pdirs, so we can share the pdir of
    // the blocked proc to read it's content.
    dumper_p->address_space = p->address_space;

    resched();
}

/**
 * The function maps kernel elf file to kernelspace.
 * Called during kernel initialisation.
 */
static int dump_map_kernel_elf_file()
{
    dentry_t* kernel_file;
    file_descriptor_t fd;
    int err = vfs_resolve_path("/boot/kernel.bin", &kernel_file);
    if (err) {
        return err;
    }

    // Kernel file's dentry is left in an open state to protect file
    // from deletion.
    kernel_file = dentry_duplicate(kernel_file);

    err = vfs_open(kernel_file, &fd, O_RDONLY);
    if (err) {
        dentry_put(kernel_file);
        return err;
    }

    size_t elf_file_size = kernel_file->inode->size;

    kernel_file_mapping_zone = kmemzone_new(elf_file_size);
    if (!kernel_file_mapping_zone.ptr) {
        dentry_put(kernel_file);
        vfs_close(&fd);
        return -ENOMEM;
    }

    uint8_t* copy_to = (uint8_t*)kernel_file_mapping_zone.ptr;
    for (size_t read = 0; read < elf_file_size; read += READ_PER_CYCLE) {
        fd.ops->read(fd.dentry, copy_to, read, READ_PER_CYCLE);
        copy_to += READ_PER_CYCLE;
    }

    vfs_close(&fd);
    return 0;
}

/**
 * The function maps kernel elf file to kernelspace.
 * Called during kernel initialisation.
 */
int dump_prepare_kernel_data()
{
    int err = dump_map_kernel_elf_file();
    if (err) {
        return err;
    }

    void* mapped_at_ptr = (void*)kernel_file_mapping_zone.ptr;
    err = elf_check_header(mapped_at_ptr);
    if (err) {
        return err;
    }

    err = elf_find_symtab_unchecked(mapped_at_ptr, &kernel_dump_data.syms, &kernel_dump_data.symsn, &kernel_dump_data.strs);
    if (err || !kernel_dump_data.syms || !kernel_dump_data.strs) {
        return err;
    }

    kernel_dump_data.p = NULL;
    kernel_dump_data.entry_point = ((elf_header_32_t*)mapped_at_ptr)->e_entry;
    kernel_dump_data.writer = dump_writer;
    kernel_dump_data.sym_resolver = elf_find_function_in_symtab;

    return 0;
}

int dump_kernel(const char* err)
{
    if (unlikely(!kernel_file_mapping_zone.ptr)) {
        dump_writer(err);
        return -1;
    }
    return dump_kernel_impl(&kernel_dump_data, err);
}

int dump_kernel_from_tf(const char* err, trapframe_t* tf)
{
    if (unlikely(!kernel_file_mapping_zone.ptr)) {
        dump_writer(err);
        return -1;
    }
    return dump_kernel_impl_from_tf(&kernel_dump_data, err, tf);
}