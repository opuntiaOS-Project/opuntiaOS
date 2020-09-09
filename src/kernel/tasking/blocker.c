/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <tasking/thread.h>
#include <tasking/sched.h>

int should_unblock_join_block(thread_t* thread)
{
    // TODO: Add more checks here.
    if (thread->joinee->status == THREAD_DYING || thread->joinee->status == THREAD_DEAD) {
        return 1;
    }
    return 0;
}

int init_join_blocker(thread_t* thread)
{
    thread->status = THREAD_BLOCKED;
    thread->blocker.reason = BLOCKER_JOIN;
    thread->blocker.should_unblock = should_unblock_join_block;
    thread->blocker.should_unblock_for_signal = true;
    sched_dequeue(thread);
    return 0;
}


int should_unblock_read_block(thread_t* thread)
{
    return thread->blocker_fd->ops->can_read(thread->blocker_fd->dentry);
}

int init_read_blocker(thread_t* thread, file_descriptor_t* bfd)
{
    thread->blocker_fd = bfd;
    thread->status = THREAD_BLOCKED;
    thread->blocker.reason = BLOCKER_JOIN;
    thread->blocker.should_unblock = should_unblock_read_block;
    thread->blocker.should_unblock_for_signal = true;
    sched_dequeue(thread);
    return 0;
}