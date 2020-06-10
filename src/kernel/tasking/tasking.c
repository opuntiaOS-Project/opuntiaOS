/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <fs/vfs.h>
#include <mem/kmalloc.h>
#include <tasking/sched.h>
#include <tasking/tasking.h>
#include <x86/gdt.h>
#include <x86/idt.h>
#include <x86/tss.h>

#define FL_IF 0x00000200

static int nxtpid = 1;

extern void trap_return();
extern void switch_contexts(context_t** old, context_t* new);

/**
 * CPU FUNCTIONS
 */

/* switching the page dir and tss to the current proc */
void switchuvm(proc_t* p)
{
    gdt[SEG_TSS] = SEG_BG(SEGTSS_TYPE, &tss, sizeof(tss) - 1, 0);
    tss.esp0 = (uint32_t)(p->kstack.start + VMM_PAGE_SIZE);
    tss.ss0 = (SEG_KDATA << 3);
    // tss.iomap_offset = 0xffff;
    active_proc = p;
    ltr(SEG_TSS << 3);
    vmm_switch_pdir(p->pdir);
}

/**
 * used to jump to trapend
 * the jump will start the process
 */
void _tasking_jumper()
{
    return;
}

/**
 * TASK LOADING FUNCTIONS
 */

static int _tasking_load_bin(pdirectory_t* pdir, file_descriptor_t* fd)
{
    uint8_t* prog = kmalloc(fd->dentry->inode->size);
    fd->ops->read(fd->dentry, prog, 0, fd->dentry->inode->size);
    vmm_copy_to_pdir(pdir, prog, 0, fd->dentry->inode->size);
    return 0;
}

static int _tasking_load(proc_t* proc, const char* path)
{
    dentry_t* file;
    file_descriptor_t fd;

    if (vfs_resolve_path_start_from(proc->cwd, path, &file) < 0) {
        return -1;
    }
    if (vfs_open(file, &fd)) {
        return -1;
    }
    int ret = _tasking_load_bin(proc->pdir, &fd);

    proc->cwd = dentry_get_parent(file);
    vfs_close(&fd);
    return ret;
}

static void _tasking_copy_proc(proc_t* new_proc)
{
    memcpy((void*)new_proc->tf, (void*)active_proc->tf, sizeof(trapframe_t));
    new_proc->cwd = dentry_duplicate(active_proc->cwd);
}

proc_t* tasking_get_active_proc()
{
    return active_proc;
}

static proc_t* _tasking_alloc_proc()
{
    proc_t* p = &proc[nxt_proc++];
    proc_prepare(p);
    p->pid = nxtpid++;

    char* sp = (char*)(p->kstack.start + VMM_PAGE_SIZE);

    /* setting trapframe in kernel stack */
    sp -= sizeof(*p->tf);
    p->tf = (trapframe_t*)sp;

    /* setting return point in kernel stack */
    sp -= 4;
    *(uint32_t*)sp = (uint32_t)trap_return;

    /* setting context in kernel stack */
    sp -= sizeof(*p->context);
    p->context = (context_t*)sp;

    /* setting init data */
    memset((void*)p->context, 0, sizeof(*p->context));
    p->context->eip = (uint32_t)_tasking_jumper;
    memset((void*)p->tf, 0, sizeof(*p->tf));

    return p;
}

/**
 * Start init proccess
 * All others processes will fork
 */
void tasking_start_init_proc()
{
    proc_t* p = _tasking_alloc_proc();

    /* creating new pdir */
    p->pdir = vmm_new_user_pdir();

    if (_tasking_load(p, "/boot/init") < 0) {
        kprintf("Can't load init proc");
        while (1) {
        }
    }

    /* setting init trapframe */
    p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
    p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
    p->tf->es = p->tf->ds;
    p->tf->ss = p->tf->ds;
    p->tf->eflags = FL_IF;
    p->tf->ebp = VMM_PAGE_SIZE;
    p->tf->esp = VMM_PAGE_SIZE;
    p->tf->eip = 0;

    /* stub context to switch for the first time */
    context_t stub_cntx;
    context_t* stub_cntx_ptr = &stub_cntx;

    /* switch to init proc */
    switchuvm(p);
    switch_contexts(&stub_cntx_ptr, p->context);
}

/**
 * TASKING RELATED FUNCTIONS
 */

void tasking_init()
{
    nxt_proc = 0;
    signal_init();
}

/**
 * SYSCALL IMPLEMENTATION
 */

/* Syscall */
void tasking_fork(trapframe_t* tf)
{
    proc_t* new_proc = _tasking_alloc_proc();
    new_proc->pdir = vmm_new_forked_user_pdir();

    /* copying data from proc to new proc */
    _tasking_copy_proc(new_proc);

    /* setting output */
    new_proc->tf->eax = 0;
    active_proc->tf->eax = new_proc->pid;

    /* switch to forked proc */
    switchuvm(new_proc);
    switch_contexts(&active_proc->context, new_proc->context);
}

/* Syscall */
/* TODO: Posix & zeroing-on-demand */
void tasking_exec(trapframe_t* tf)
{
    proc_t* proc = tasking_get_active_proc();
    char* launch_path = (char*)proc->tf->ecx; // for now let's think that our string is at ecx
    if (_tasking_load(proc, launch_path) < 0) {
        proc->tf->eax = -1;
        return;
    }
    proc->tf->ebp = VMM_PAGE_SIZE;
    proc->tf->esp = VMM_PAGE_SIZE;
    proc->tf->eip = 0;
}

/* Syscall */
void tasking_exit(trapframe_t* tf)
{
    proc_t* proc = tasking_get_active_proc();
    kprintf("Task %d exit with code: %d\n", proc->pid, tf->ebx);
    proc_free(proc);
    presched_no_context();
}