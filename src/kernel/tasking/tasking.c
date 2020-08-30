/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <errno.h>
#include <fs/vfs.h>
#include <mem/kmalloc.h>
#include <tasking/sched.h>
#include <tasking/tasking.h>
#include <tty/tty.h>
#include <x86/common.h>
#include <x86/gdt.h>
#include <x86/idt.h>
#include <x86/tss.h>

static int nxtpid = 1;
proc_t proc[MAX_PROCESS_COUNT];
uint32_t nxt_proc;
uint32_t ended_proc;

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
    sti();
    return;
}

/**
 * TASK LOADING FUNCTIONS
 */

static int _tasking_load_bin(proc_t* p, file_descriptor_t* fd)
{
    uint32_t code_size = fd->dentry->inode->size;
    proc_zone_t* code_zone = proc_new_random_zone(p, code_size);
    code_zone->type = ZONE_TYPE_CODE;
    code_zone->flags |= ZONE_READABLE | ZONE_EXECUTABLE;

    /* THIS IS FOR BSS WHICH COULD BE IN THIS ZONE */
    code_zone->flags |= ZONE_WRITABLE;

    proc_zone_t* bss_zone = proc_new_random_zone(p, 1024);
    bss_zone->type = ZONE_TYPE_DATA;
    bss_zone->flags |= ZONE_READABLE | ZONE_WRITABLE;

    proc_zone_t* stack_zone = proc_new_random_zone_backward(p, VMM_PAGE_SIZE);
    stack_zone->type = ZONE_TYPE_STACK;
    stack_zone->flags |= ZONE_READABLE | ZONE_WRITABLE;


    /* Copying an exec code */
    uint8_t* prog = kmalloc(fd->dentry->inode->size);
    fd->ops->read(fd->dentry, prog, 0, fd->dentry->inode->size);
    vmm_copy_to_pdir(p->pdir, prog, code_zone->start, fd->dentry->inode->size);

    /* Setting registers */
    proc_setup_segment_regs(p);
    p->tf->ebp = stack_zone->start + VMM_PAGE_SIZE;
    p->tf->esp = p->tf->ebp;
    p->tf->eip = code_zone->start;

    kfree(prog);

    return 0;
}

static int _tasking_load(proc_t* proc, const char* path)
{
    dentry_t* file;
    file_descriptor_t fd;

    if (vfs_resolve_path_start_from(proc->cwd, path, &file) < 0) {
        return -ENOENT;
    }
    if (vfs_open(file, &fd) < 0) {
        dentry_put(file);
        return -ENOENT;
    }

    /* Put it back, since now we have a new cwd */
    if (proc->cwd) {
        dentry_put(proc->cwd);
    }

    int ret = _tasking_load_bin(proc, &fd);

    proc->cwd = dentry_get_parent(file);

    dentry_put(file);
    vfs_close(&fd);
    return ret;
}

static void _tasking_copy_proc(proc_t* new_proc)
{
    memcpy((void*)new_proc->tf, (void*)active_proc->tf, sizeof(trapframe_t));
    new_proc->cwd = dentry_duplicate(active_proc->cwd);
    new_proc->tty = active_proc->tty;

    /* TODO: change the size in advance */
    for (int i = 0; i < active_proc->zones.size; i++) {
        proc_zone_t* zone_to_copy = (proc_zone_t*)dynamic_array_get(&active_proc->zones, i);
        dynamic_array_push(&new_proc->zones, zone_to_copy);
    }

    if (active_proc->fds) {
        for (int i = 0; i < MAX_OPENED_FILES; i++) {
            if (active_proc->fds[i].dentry) {
                file_descriptor_t* fd = &new_proc->fds[i];
                vfs_open(active_proc->fds[i].dentry, fd);
            }
        }
    }
}

proc_t* tasking_get_active_proc()
{
    return active_proc;
}

proc_t* tasking_get_proc(uint32_t pid)
{
    proc_t* p;
    for (int i = 0; i < nxt_proc; i++) {
        p = &proc[i];
        if (p->pid == pid) {
            return p;
        }
    }
    return 0;
}

proc_t* tasking_get_proc_by_pdir(pdirectory_t* pdir)
{
    proc_t* p;
    for (int i = 0; i < nxt_proc; i++) {
        p = &proc[i];
        if (p->pdir == pdir) {
            return p;
        }
    }
    return 0;
}

static proc_t* _tasking_alloc_proc()
{
    proc_t* p = &proc[nxt_proc++];
    p->pid = nxtpid++;
    proc_setup(p);
    proc_setup_kstack(p);
    return p;
}

static proc_t* _tasking_alloc_kernel_thread(void* entry_point)
{
    proc_t* p = &proc[nxt_proc++];
    p->pid = nxtpid++;
    kthread_setup(p);
    proc_setup_kstack(p);
    kthread_setup_regs(p, entry_point);
    return p;
}

/**
 * Start init proccess
 * All others processes will fork
 */
void tasking_start_init_proc()
{
    proc_t* p = _tasking_alloc_proc();
    proc_setup_tty(p, tty_new());

    /* creating new pdir */
    p->pdir = vmm_new_user_pdir();

    if (_tasking_load(p, "/boot/init") < 0) {
        kprintf("Failed to load init proc");
        p->status = PROC_INVALID;
        while (1) { }
    }
    p->status = PROC_RUNNING;
}

int tasking_create_kernel_thread(void* entry_point)
{
    proc_t* p = _tasking_alloc_kernel_thread(entry_point);
    p->pdir = vmm_get_kernel_pdir();
    p->status = PROC_RUNNING;
    return 0;
}

/**
 * TASKING RELATED FUNCTIONS
 */

void tasking_init()
{
    nxt_proc = 0;
    ended_proc = 1;
    signal_init();
}

void tasking_die(proc_t* p)
{
    p->status = PROC_DYING;
    ended_proc++;
}

void tasking_kill_dying()
{
    proc_t* p;
    for (int i = 0; i < nxt_proc; i++) {
        p = &proc[i];
        if (p->status == PROC_DYING) {
            proc_free(p);
            p->status = PROC_DEAD;
        }
    }
}

/**
 * SYSCALL IMPLEMENTATION
 */

/* Syscall */
void tasking_fork(trapframe_t* tf)
{
    proc_t* new_proc = _tasking_alloc_proc();
    new_proc->pdir = vmm_new_forked_user_pdir();
    new_proc->status = active_proc->status;

    /* copying data from proc to new proc */
    _tasking_copy_proc(new_proc);

    /* setting output */
    new_proc->tf->eax = 0;
    active_proc->tf->eax = new_proc->pid;

    /*  After copying the task we need to flush tlb. To do that we need
        to reload cr3 register with a new pdir. To not waste our resources
        we will simply run other process and of course pdir will be refreshed. */
    presched();
}

static int _tasking_do_exec(proc_t* p, const char* path, int argc, char** argv, char** env)
{
    int res = _tasking_load(p, path);
    proc_fill_up_stack(p, argc, argv, env);
    return res;
}

/* Syscall */
/* TODO: Posix & zeroing-on-demand */
int tasking_exec(const char* path, const char** argv, const char** env)
{
    proc_t* p = tasking_get_active_proc();
    char* kpath = 0;
    int kargc = 0;
    char** kargv = 0;
    char** kenv = 0;

    if (!str_validate_len(path, 128)) {
        return -EINVAL;
    }
    kpath = kmem_bring_to_kernel(path, strlen(path) + 1);

    if (argv) {
        if (!ptrarr_validate_len(argv, 128)) {
            return -EINVAL;
        }
        kargc = ptrarr_len(argv);
        
        /* Validating arguments size */
        uint32_t data_len = 0;
        for (int argi = 0; argi < kargc; argi++) {
            if (!str_validate_len(argv[argi], 128)) {
                return -EINVAL;
            }
            data_len += strlen(argv[argi]) + 1;
            if (data_len > 128) {
                return -EINVAL;
            }
        }

        kargv = kmem_bring_to_kernel_ptrarr(argv, kargc);
    }
    

    /* Cleaning proc */
    dynamic_array_clear(&p->zones);

    int ret = _tasking_do_exec(p, kpath, kargc, kargv, 0);

    kfree(kpath);
    for (int argi = 0; argi < kargc; argi++) {
        kfree(kargv[argi]);
    }
    kfree(kargv);

    return ret;
}

int tasking_waitpid(int pid)
{
    proc_t* proc = tasking_get_active_proc();
    proc_t* joinee_proc = tasking_get_proc(pid);
    if (!joinee_proc) {
        return -ESRCH;
    }
    proc->joinee = joinee_proc;
    joinee_proc->joiner = proc;
    init_join_blocker(proc);
    presched();
    return 0;
}

/* Syscall */
void tasking_exit(int exit_code)
{
    proc_t* proc = tasking_get_active_proc();
    proc->exit_code = exit_code;
    tasking_die(proc);
    presched_no_context();
}