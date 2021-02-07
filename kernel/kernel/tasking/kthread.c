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
#include <utils.h>

/**
 * For x86 we can use the same stack both for kernel and kthread operations.
 * For arm we need use 2 stacks: one stack for svc mode and one for sys mode, so
 * we create a bigger stack (2 * page_size) to fit 2 stacks required for arm.
 */
#ifdef __i386__
#define KSTACK_ZONE_SIZE VMM_PAGE_SIZE
#define KSTACK_TOP VMM_PAGE_SIZE
#define USTACK_TOP VMM_PAGE_SIZE
#elif __arm__
#define KSTACK_ZONE_SIZE (2 * VMM_PAGE_SIZE)
#define KSTACK_TOP VMM_PAGE_SIZE
#define USTACK_TOP (2 * VMM_PAGE_SIZE)
#endif

extern void trap_return();
extern void _tasking_jumper();

extern int _thread_setup_kstack(thread_t* thread, uint32_t esp);
int kthread_setup(proc_t* p)
{
    p->pid = proc_alloc_pid();
    p->pgid = p->pid;
    p->is_kthread = true;
    /* allocating kernel stack */
    p->main_thread = proc_alloc_thread();
    p->main_thread->tid = p->pid;
    p->main_thread->process = p;

    p->main_thread->kstack = zoner_new_zone(KSTACK_ZONE_SIZE);
    if (!p->main_thread->kstack.start) {
        return -ENOMEM;
    }
    _thread_setup_kstack(p->main_thread, p->main_thread->kstack.start + KSTACK_TOP);

    /* setting current work directory */
    p->cwd = 0;

    p->fds = 0;

    /* setting signal handlers to 0 */
    p->main_thread->signals_mask = 0x0; /* All signals are disabled. */
    p->main_thread->pending_signals_mask = 0x0;
    memset((void*)p->main_thread->signal_handlers, 0, sizeof(p->main_thread->signal_handlers));

    return 0;
}

int kthread_setup_regs(proc_t* p, void* entry_point)
{
    tf_setup_as_kernel_thread(p->main_thread->tf);
    uint32_t stack = (p->main_thread->kstack.start + USTACK_TOP);
    set_base_pointer(p->main_thread->tf, stack);
    set_stack_pointer(p->main_thread->tf, stack);
    set_instruction_pointer(p->main_thread->tf, (uint32_t)entry_point);
    return 0;
}

int kthread_free(proc_t* p)
{
    /* proc_free can free kthreads too) */
    proc_free(p);
}