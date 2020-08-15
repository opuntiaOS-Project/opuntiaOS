/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <fs/vfs.h>
#include <mem/kmalloc.h>
#include <tasking/proc.h>
#include <tty/tty.h>
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
        return -1;
    }

    /* setting current work directory */
    p->cwd = 0;

    /* allocating space for open files */
    p->fds = kmalloc(MAX_OPENED_FILES * sizeof(file_descriptor_t));
    if (!p->fds) {
        return -1;
    }
    memset((void*)p->fds, 0, MAX_OPENED_FILES * sizeof(file_descriptor_t));

    /* setting signal handlers to 0 */
    p->signals_mask = 0xffffffff; /* for now all signals are legal */
    p->pending_signals_mask = 0;
    memset((void*)p->signal_handlers, 0, sizeof(p->signal_handlers));

    /* setting up zones */
    if (dynamic_array_init_of_size(&p->zones, sizeof(proc_zone_t), 8) != 0) {
        return -1;
    }

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
        return -1;
    }
    int res = vfs_open(tty_dentry, fd0);
    res = vfs_open(tty_dentry, fd1);
    dentry_put(tty_dentry);
    return 0;
}

int kthread_setup(proc_t* p)
{
    p->is_kthread = true;
    /* allocating kernel stack */
    p->kstack = zoner_new_zone(VMM_PAGE_SIZE);
    if (!p->kstack.start) {
        return -1;
    }

    /* setting current work directory */
    p->cwd = 0;

    p->fds = 0;

    /* setting signal handlers to 0 */
    p->signals_mask = 0x0; /* All signals are disabled. */
    p->pending_signals_mask = 0x0;
    memset((void*)p->signal_handlers, 0, sizeof(p->signal_handlers));

    return 0;
}

int kthread_setup_regs(proc_t* p, void* entry_point)
{
    zone_t stack = zoner_new_zone(VMM_PAGE_SIZE);
    if (!stack.start) {
        return -1;
    }

    kthread_segregs_setup(p);
    p->tf->ebp = (stack.start + VMM_PAGE_SIZE);
    p->tf->esp = p->tf->ebp;
    p->tf->eip = (uint32_t)entry_point;
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

void proc_segregs_setup(proc_t* p)
{
    p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
    p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
    p->tf->es = p->tf->ds;
    p->tf->ss = p->tf->ds;
    p->tf->eflags = FL_IF;
}

void kthread_segregs_setup(proc_t* p)
{
    p->tf->cs = (SEG_KCODE << 3);
    p->tf->ds = (SEG_KDATA << 3);
    p->tf->es = p->tf->ds;
    p->tf->ss = p->tf->ds;
    p->tf->eflags = FL_IF;
}

int proc_free(proc_t* p)
{
    if (p->status == PROC_DEAD || p->status == PROC_INVALID || p->pid == 0) {
        return -1;
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

    zoner_free_zone(p->kstack);

    if (!p->is_kthread) {
        vmm_free_pdir(p->pdir);
    }

    return 0;
}

/**
 * PROC FD FUNCTIONS
 */

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
 * PROC ZONING
 */

static inline bool _proc_zones_intersect(uint32_t start1, uint32_t size1, uint32_t start2, uint32_t size2)
{
    uint32_t end1 = start1 + size1 - 1;
    uint32_t end2 = start2 + size2 - 1;
    return (start1 <= start2 && start2 <= end1) || (start1 <= end2 && end2 <= end1) || (start2 <= start1 && start1 <= end2) || (start2 <= end1 && end1 <= end2);
}

static inline bool _proc_can_add_zone(proc_t* proc, uint32_t start, uint32_t len)
{
    uint32_t zones_count = proc->zones.size;

    for (uint32_t i = 0; i < zones_count; i++) {
        proc_zone_t* zone = (proc_zone_t*)dynamic_array_get(&proc->zones, i);
        if (_proc_zones_intersect(start, len, zone->start, zone->len)) {
            return false;
        }
    }

    return true;
}

proc_zone_t* proc_new_zone(proc_t* proc, uint32_t start, uint32_t len)
{
    proc_zone_t new_zone;
    new_zone.start = start;
    new_zone.len = len;

    if (_proc_can_add_zone(proc, start, len)) {
        if (dynamic_array_push(&proc->zones, &new_zone) != 0) {
            return 0;
        }
        return (proc_zone_t*)dynamic_array_get(&proc->zones, proc->zones.size - 1);
    }

    return 0;
}

/* FIXME: Think of more efficient way */
proc_zone_t* proc_new_random_zone(proc_t* proc, uint32_t len)
{
    uint32_t zones_count = proc->zones.size;

    /* Check if we can put it at the beginning */
    proc_zone_t* ret = proc_new_zone(proc, 0, len);
    if (ret) {
        return ret;
    }

    uint32_t min_start = 0xffffffff;

    for (uint32_t i = 0; i < zones_count; i++) {
        proc_zone_t* zone = (proc_zone_t*)dynamic_array_get(&proc->zones, i);
        if (_proc_can_add_zone(proc, zone->start + zone->len, len)) {
            if (min_start > zone->start + zone->len) {
                min_start = zone->start + zone->len;
            }
        }
    }

    if (min_start == 0xffffffff) {
        return 0;
    }

    return proc_new_zone(proc, min_start, len);
}

/* FIXME: Think of more efficient way */
proc_zone_t* proc_new_random_zone_backward(proc_t* proc, uint32_t len)
{
    uint32_t zones_count = proc->zones.size;

    /* Check if we can put it at the end */
    proc_zone_t* ret = proc_new_zone(proc, KERNEL_BASE - len, len);
    if (ret) {
        return ret;
    }

    uint32_t max_end = 0;

    for (uint32_t i = 0; i < zones_count; i++) {
        proc_zone_t* zone = (proc_zone_t*)dynamic_array_get(&proc->zones, i);
        if (_proc_can_add_zone(proc, zone->start - len, len)) {
            if (max_end < zone->start) {
                max_end = zone->start;
            }
        }
    }

    if (max_end == 0) {
        return 0;
    }

    return proc_new_zone(proc, max_end - len, len);
}