/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <tasking/sched.h>
#include <tasking/thread.h>
#include <time/time_manager.h>
#include <log.h>

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
    resched();
    return 0;
}

int should_unblock_read_block(thread_t* thread)
{
    return thread->blocker_fd->ops->can_read(thread->blocker_fd->dentry, thread->blocker_fd->offset);
}

int init_read_blocker(thread_t* thread, file_descriptor_t* bfd)
{
    thread->blocker_fd = bfd;

    if (should_unblock_read_block(thread)) {
        return 0;
    }

    thread->status = THREAD_BLOCKED;
    thread->blocker.reason = BLOCKER_READ;
    thread->blocker.should_unblock = should_unblock_read_block;
    thread->blocker.should_unblock_for_signal = true;
    sched_dequeue(thread);
    resched();
    return 0;
}

int should_unblock_write_block(thread_t* thread)
{
    return thread->blocker_fd->ops->can_write(thread->blocker_fd->dentry, thread->blocker_fd->offset);
}

int init_write_blocker(thread_t* thread, file_descriptor_t* bfd)
{
    thread->blocker_fd = bfd;

    if (should_unblock_write_block(thread)) {
        return 0;
    }

    thread->status = THREAD_BLOCKED;
    thread->blocker.reason = BLOCKER_WRITE;
    thread->blocker.should_unblock = should_unblock_write_block;
    thread->blocker.should_unblock_for_signal = true;
    sched_dequeue(thread);
    resched();
    return 0;
}

int should_unblock_sleep_block(thread_t* thread)
{
    return thread->unblock_time <= timeman_now();
}

int init_sleep_blocker(thread_t* thread, uint32_t time)
{
    thread->unblock_time = timeman_now() + time;

    if (should_unblock_sleep_block(thread)) {
        return 0;
    }

    thread->status = THREAD_BLOCKED;
    thread->blocker.reason = BLOCKER_SLEEP;
    thread->blocker.should_unblock = should_unblock_sleep_block;
    thread->blocker.should_unblock_for_signal = true;
    sched_dequeue(thread);
    resched();
    return 0;
}

int should_unblock_select_block(thread_t* thread)
{
    if (thread->unblock_time != 0 && thread->unblock_time <= timeman_now()) {
        return true;
    }

    file_descriptor_t* fd;
    for (int i = 0; i < thread->nfds; i++) {
        if (FD_ISSET(i, &thread->readfds)) {
            fd = proc_get_fd(thread->process, i);
            if (fd->ops->can_read(fd->dentry, fd->offset)) {
                return true;
            }
        }
    }

    for (int i = 0; i < thread->nfds; i++) {
        if (FD_ISSET(i, &thread->writefds)) {
            fd = proc_get_fd(thread->process, i);
            if (fd->ops->can_write(fd->dentry, fd->offset)) {
                return true;
            }
        }
    }
    return false;
}

int init_select_blocker(thread_t* thread, int nfds, fd_set_t* readfds, fd_set_t* writefds, fd_set_t* exceptfds, timeval_t* timeout)
{
    FD_ZERO(&(thread->readfds));
    FD_ZERO(&(thread->writefds));
    FD_ZERO(&(thread->exceptfds));
    thread->unblock_time = 0;

    if (readfds) {
        thread->readfds = *readfds;
    }
    if (writefds) {
        thread->writefds = *writefds;
    }
    if (exceptfds) {
        thread->exceptfds = *exceptfds;
    }
    if (timeout) {
        thread->unblock_time = timeman_now() + timeout->tv_sec;
    }
    thread->nfds = nfds;

    if (should_unblock_select_block(thread)) {
        return 0;
    }

    thread->status = THREAD_BLOCKED;
    thread->blocker.reason = BLOCKER_SELECT;
    thread->blocker.should_unblock = should_unblock_select_block;
    thread->blocker.should_unblock_for_signal = true;
    sched_dequeue(thread);
    resched();
    return 0;
}
