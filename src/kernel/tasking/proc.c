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
#include <tasking/proc.h>
#include <io/tty/tty.h>
#include <x86/gdt.h>
#include <x86/tss.h>

extern void trap_return();
extern void _tasking_jumper();

/**
 * INIT FUNCTIONS
 */

int proc_setup(proc_t* p)
{
    p->is_kthread = false;
    /* allocating kernel stack */
    p->kstack = zoner_new_zone(VMM_PAGE_SIZE);
    if (!p->kstack.start) {
        return -ENOMEM;
    }

    /* setting current work directory */
    p->cwd = 0;

    /* allocating space for open files */
    p->fds = kmalloc(MAX_OPENED_FILES * sizeof(file_descriptor_t));
    if (!p->fds) {
        return -ENOMEM;
    }
    memset((void*)p->fds, 0, MAX_OPENED_FILES * sizeof(file_descriptor_t));

    /* setting signal handlers to 0 */
    p->signals_mask = 0xffffffff; /* for now all signals are legal */
    p->pending_signals_mask = 0;
    memset((void*)p->signal_handlers, 0, sizeof(p->signal_handlers));

    /* setting up zones */
    if (dynamic_array_init_of_size(&p->zones, sizeof(proc_zone_t), 8) != 0) {
        return -ENOMEM;
    }

    p->prio = 6;

    return 0;
}

int proc_setup_tty(proc_t* p, tty_entry_t* tty)
{
    file_descriptor_t* fd0 = &p->fds[0];
    file_descriptor_t* fd1 = &p->fds[1];
    p->tty = tty;

    char* path_to_tty = "dev/tty ";
    path_to_tty[7] = tty->id + '0';
    dentry_t* tty_dentry;
    if (vfs_resolve_path(path_to_tty, &tty_dentry) < 0) {
        return -ENOENT;
    }
    int res = vfs_open(tty_dentry, fd0);
    res = vfs_open(tty_dentry, fd1);
    dentry_put(tty_dentry);
    return 0;
}

int proc_setup_kstack(proc_t* p)
{
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

    return 0;
}

void proc_setup_segment_regs(proc_t* p)
{
    p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
    p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
    p->tf->es = p->tf->ds;
    p->tf->ss = p->tf->ds;
    p->tf->eflags = FL_IF;
}

/**
 * PROC STACK FUNCTIONS
 */

static inline void _proc_push_to_user_stack(proc_t* proc, uint32_t value)
{
    proc->tf->esp -= 4;
    *((uint32_t*)proc->tf->esp) = value;
}

static inline uint32_t _proc_pop_from_user_stack(proc_t* proc)
{
    uint32_t val = *((uint32_t*)proc->tf->esp);
    proc->tf->esp += 4;
    return val;
}

static inline void _proc_simulate_push_to_user_stack(proc_t* proc)
{
    proc->tf->esp -= 4;
}

static inline void _proc_simulate_pop_from_user_stack(proc_t* proc)
{
    proc->tf->esp += 4;
}

int proc_fill_up_stack(proc_t* p, int argc, char** argv, char** env)
{
    /* TODO: Add env */
    uint32_t argv_data_size = 0;
    for (int i = 0; i < argc; i++) {
        argv_data_size += strlen(argv[i]) + 1;
    }

    if (argv_data_size % 4) {
        argv_data_size += 4 - (argv_data_size % 4);
    }
    
    uint32_t size_in_stack = argv_data_size + (argc + 1) * sizeof(char*) + sizeof(argc) + sizeof(char*);
    int* tmp_buf = (int*)kmalloc(size_in_stack);
    if (!tmp_buf) {
        return -EAGAIN;
    }
    memset((void*)tmp_buf, 0, size_in_stack);

    char* tmp_buf_ptr = ((char*)tmp_buf) + size_in_stack;
    char* tmp_buf_data_ptr = tmp_buf_ptr - argv_data_size;
    uint32_t* tmp_buf_array_ptr = (uint32_t*)((char*)tmp_buf_data_ptr - (argc + 1) * sizeof(char*));
    int* tmp_buf_argv_ptr = (int*)((char*)tmp_buf_array_ptr - sizeof(char*));
    int* tmp_buf_argc_ptr = (int*)((char*)tmp_buf_argv_ptr - sizeof(int));
    
    uint32_t data_esp = p->tf->esp - argv_data_size;
    uint32_t array_esp = data_esp - (argc + 1) * sizeof(char*);
    uint32_t argv_esp = array_esp - 4;
    uint32_t argc_esp = argv_esp - 4;

    for (int i = argc - 1; i >= 0; i--) {
        uint32_t len = strlen(argv[i]);
        tmp_buf_ptr -= len + 1;
        p->tf->esp -= len + 1;
        memcpy(tmp_buf_ptr, argv[i], len);
        tmp_buf_ptr[len] = 0;

        tmp_buf_array_ptr[i] = p->tf->esp;
    }
    tmp_buf_array_ptr[argc] = 0;

    *tmp_buf_argv_ptr = array_esp;
    *tmp_buf_argc_ptr = argc;

    p->tf->esp = argc_esp;

    vmm_copy_to_pdir(p->pdir, (uint8_t*)tmp_buf, p->tf->esp, size_in_stack);

    kfree(tmp_buf);

    return 0;
}

int proc_free(proc_t* p)
{
    if (p->status == PROC_DEAD || p->status == PROC_INVALID || p->pid == 0) {
        return -ESRCH;
    }

    p->pid = 0;
    p->status = PROC_DEAD;

    /* closing opend fds */
    if (p->fds) {
        for (int i = 0; i < MAX_OPENED_FILES; i++) {
            if (p->fds[i].dentry) {
                /* think as an active fd */
                vfs_close(&p->fds[i]);
            }
        }
        kfree(p->fds);
    }

    if (p->cwd) {
        dentry_put(p->cwd);
    }

    dynamic_array_free(&p->zones);

    /* Key parts deletion. After that line you can't work with this process. */
    zoner_free_zone(p->kstack);

    if (!p->is_kthread) {
        vmm_free_pdir(p->pdir);
    }

    return 0;
}

/**
 * PROC FD FUNCTIONS
 */

int proc_get_fd_id(proc_t* proc, file_descriptor_t* fd)
{
    /* Calculating id with pointers */
    uint32_t start = (uint32_t)proc->fds;
    uint32_t fd_ptr = (uint32_t)fd;
    fd_ptr -= start;
    int fd_res = fd_ptr / sizeof(file_descriptor_t);
    if (!(fd_ptr % sizeof(file_descriptor_t))) {
        return fd_res;
    }
    return -1;
}

file_descriptor_t* proc_get_free_fd(proc_t* proc)
{
    for (int i = 0; i < MAX_OPENED_FILES; i++) {
        if (!proc->fds[i].dentry) {
            return &proc->fds[i];
        }
    }
}

file_descriptor_t* proc_get_fd(proc_t* proc, uint32_t index)
{
    if (index >= MAX_OPENED_FILES) {
        return 0;
    }

    if (!proc->fds[index].dentry) {
        return 0;
    }

    return &proc->fds[index];
}

/**
 * PROC DEBUG FUNCTIONS
 */

int proc_dump_frame(proc_t* p)
{
    for (uint32_t i = p->tf->esp; i < p->tf->ebp; i++) {
        uint8_t byte = *(uint8_t*)i;
        uint32_t b32 = (uint32_t)byte;
        kprintf("%x - %x\n", i, b32);
    }
}