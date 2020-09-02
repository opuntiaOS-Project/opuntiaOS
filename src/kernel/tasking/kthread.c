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

int kthread_setup(proc_t* p)
{
    p->is_kthread = true;
    /* allocating kernel stack */
    p->kstack = zoner_new_zone(VMM_PAGE_SIZE);
    if (!p->kstack.start) {
        return -ENOMEM;
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
        return -ENOMEM;
    }

    kthread_setup_segment_regs(p);
    p->tf->ebp = (stack.start + VMM_PAGE_SIZE);
    p->tf->esp = p->tf->ebp;
    p->tf->eip = (uint32_t)entry_point;
    return 0;
}

void kthread_setup_segment_regs(proc_t* p)
{
    p->tf->cs = (SEG_KCODE << 3);
    p->tf->ds = (SEG_KDATA << 3);
    p->tf->es = p->tf->ds;
    p->tf->ss = p->tf->ds;
    p->tf->eflags = FL_IF;
}

int kthread_free(proc_t* p)
{
    /* proc_free can free kthreads too) */
    proc_free(p);
}