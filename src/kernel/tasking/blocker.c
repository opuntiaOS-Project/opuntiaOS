/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <tasking/proc.h>
#include <tasking/sched.h>

int should_unblock_join_block(proc_t* p)
{
    // TODO: Add more checks here.
    if (p->joinee->status == PROC_DEAD) {
        return 1;
    }
    return 0;
}

int init_join_blocker(proc_t* p)
{
    p->status = PROC_BLOCKED;
    p->blocker.reason = BLOCKER_JOIN;
    p->blocker.should_unblock = should_unblock_join_block;
    sched_dequeue(p);
    return 0;
}


int should_unblock_read_block(proc_t* p)
{
    return p->blocker_fd->ops->can_read(p->blocker_fd->dentry);
}

int init_read_blocker(proc_t* p, file_descriptor_t* bfd)
{
    p->blocker_fd = bfd;
    p->status = PROC_BLOCKED;
    p->blocker.reason = BLOCKER_JOIN;
    p->blocker.should_unblock = should_unblock_read_block;
    sched_dequeue(p);
    return 0;
}