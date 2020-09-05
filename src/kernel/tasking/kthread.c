/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <errno.h>
#include <fs/vfs.h>
#include <io/tty/tty.h>
#include <mem/kmalloc.h>
#include <tasking/proc.h>
#include <tasking/thread.h>
#include <x86/gdt.h>
#include <x86/tss.h>

extern void trap_return();
extern void _tasking_jumper();

extern int _thread_setup_kstack(thread_t* thread);
int kthread_setup(proc_t* p)
{
    p->is_kthread = true;
    /* allocating kernel stack */
    p->threads = proc_alloc_thread();
    p->threads->tid = p->pid;
    p->threads->process = p;

    p->threads->kstack = zoner_new_zone(VMM_PAGE_SIZE);
    if (!p->threads->kstack.start) {
        return -ENOMEM;
    }
    _thread_setup_kstack(p->threads);

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
    p->threads->tf->ebp = (stack.start + VMM_PAGE_SIZE);
    p->threads->tf->esp = p->threads->tf->ebp;
    p->threads->tf->eip = (uint32_t)entry_point;
    return 0;
}

void kthread_setup_segment_regs(proc_t* p)
{
    p->threads->tf->cs = (SEG_KCODE << 3);
    p->threads->tf->ds = (SEG_KDATA << 3);
    p->threads->tf->es = p->threads->tf->ds;
    p->threads->tf->ss = p->threads->tf->ds;
    p->threads->tf->eflags = FL_IF;
}

int kthread_free(proc_t* p)
{
    /* proc_free can free kthreads too) */
    proc_free(p);
}