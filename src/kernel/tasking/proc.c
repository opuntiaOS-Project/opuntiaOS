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

/**
 * INIT FUNCTIONS
 */

int proc_prepare(proc_t* p)
{
    /* allocating kernel stack */
    p->kstack = zoner_new_zone(VMM_PAGE_SIZE);
    if (p->kstack.start == 0) {
        return -1;
    }

    /* setting current work directory */
    p->cwd = 0;

    /* allocating space for open files */
    p->fds = kmalloc(MAX_OPENED_FILES * sizeof(file_descriptor_t));
    if (p->fds == 0) {
        return -1;
    }

    /* setting signal handlers to 0 */
    p->signals_mask = 0xffffffff; /* for now all signals are legal */
    p->pending_signals_mask = 0;
    memset((void*)p->signal_handlers, 0, sizeof(p->signal_handlers));

    return 0;
}

int proc_free(proc_t* p)
{
    if (p->pid == 0) {
        return -1;
    }
    
    /* closing opend fds */
    for (int i = 0; i < MAX_OPENED_FILES; i++) {
        if (p->fds[i].dentry) {
            /* think as active fd */
            vfs_close(&p->fds[i]);
        }
    }

    p->pid = 0;
    kfree(p->fds);
    zoner_free_zone(p->kstack);
    dentry_put(p->cwd);
    vmm_free_pdir(p->pdir);

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

file_descriptor_t* proc_get_fd(proc_t* proc, int index)
{
    return &proc->fds[index];
}
