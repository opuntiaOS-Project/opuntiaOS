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

    /* setting up zones */
    if (dynamic_array_init(&p->zones, sizeof(proc_zone_t)) != 0) {
        return -1;
    }

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
            /* think as an active fd */
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