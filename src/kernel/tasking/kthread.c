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
    p->main_thread = proc_alloc_thread();
    p->main_thread->tid = p->pid;
    p->main_thread->process = p;

    p->main_thread->kstack = zoner_new_zone(VMM_PAGE_SIZE);
    if (!p->main_thread->kstack.start) {
        return -ENOMEM;
    }
    _thread_setup_kstack(p->main_thread);

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
    p->main_thread->tf->ebp = (stack.start + VMM_PAGE_SIZE);
    p->main_thread->tf->esp = p->main_thread->tf->ebp;
    p->main_thread->tf->eip = (uint32_t)entry_point;
    return 0;
}

void kthread_setup_segment_regs(proc_t* p)
{
    p->main_thread->tf->cs = (SEG_KCODE << 3);
    p->main_thread->tf->ds = (SEG_KDATA << 3);
    p->main_thread->tf->es = p->main_thread->tf->ds;
    p->main_thread->tf->ss = p->main_thread->tf->ds;
    p->main_thread->tf->eflags = FL_IF;
}

int kthread_free(proc_t* p)
{
    /* proc_free can free kthreads too) */
    proc_free(p);
}