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
#include <tasking/sched.h>
#include <tasking/thread.h>
#include <x86/gdt.h>
#include <x86/tss.h>

static int proc_next_pid = 1;

/* TODO: Will be removed */
thread_t thread_storage[512];
int threads_cnt = 0;

/**
 * HELPER FUNCTIONS
 */

#define FOREACH_THREAD(p)                               \
    for (int i = 0; i < threads_cnt; i++) {             \
        if (thread_storage[i].process->pid == p->pid) { \
            thread_t* thread = &thread_storage[i];

#define END_FOREACH \
    }               \
    }

thread_t* proc_alloc_thread()
{
    return &thread_storage[threads_cnt++];
}

uint32_t proc_alloc_pid()
{
    return proc_next_pid++;
}

/**
 * INIT FUNCTIONS
 */

int proc_setup(proc_t* p)
{
    p->pid = proc_alloc_pid();
    p->pgid = p->pid;
    p->is_kthread = false;

    p->main_thread = proc_alloc_thread();
    int res = thread_setup_main(p, p->main_thread);
    if (res != 0) {
        return res;
    }

    /* setting current work directory */
    p->cwd = 0;

    /* allocating space for open files */
    p->fds = kmalloc(MAX_OPENED_FILES * sizeof(file_descriptor_t));
    if (!p->fds) {
        return -ENOMEM;
    }
    memset((void*)p->fds, 0, MAX_OPENED_FILES * sizeof(file_descriptor_t));

    /* setting signal handlers to 0 */
    p->signals_mask = 0xffffffff; /* for now all signals are legal */
    p->pending_signals_mask = 0;
    memset((void*)p->signal_handlers, 0, sizeof(p->signal_handlers));

    /* setting up zones */
    if (dynamic_array_init_of_size(&p->zones, sizeof(proc_zone_t), 8) != 0) {
        return -ENOMEM;
    }

    p->status = PROC_ALIVE;
    p->prio = DEFAULT_PRIO;

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
        return -ENOENT;
    }
    int res = vfs_open(tty_dentry, fd0);
    res = vfs_open(tty_dentry, fd1);
    dentry_put(tty_dentry);
    return 0;
}

/**
 * LOAD FUNCTIONS
 */

static int _proc_load_bin(proc_t* p, file_descriptor_t* fd)
{
    uint32_t code_size = fd->dentry->inode->size;
    proc_zone_t* code_zone = proc_new_random_zone(p, code_size);
    code_zone->type = ZONE_TYPE_CODE;
    code_zone->flags |= ZONE_READABLE | ZONE_EXECUTABLE;

    /* THIS IS FOR BSS WHICH COULD BE IN THIS ZONE */
    code_zone->flags |= ZONE_WRITABLE;

    proc_zone_t* bss_zone = proc_new_random_zone(p, 1024);
    bss_zone->type = ZONE_TYPE_DATA;
    bss_zone->flags |= ZONE_READABLE | ZONE_WRITABLE;

    proc_zone_t* stack_zone = proc_new_random_zone_backward(p, VMM_PAGE_SIZE);
    stack_zone->type = ZONE_TYPE_STACK;
    stack_zone->flags |= ZONE_READABLE | ZONE_WRITABLE;

    /* Copying an exec code */
    uint8_t* prog = kmalloc(fd->dentry->inode->size);
    fd->ops->read(fd->dentry, prog, 0, fd->dentry->inode->size);
    vmm_copy_to_pdir(p->pdir, prog, code_zone->start, fd->dentry->inode->size);

    /* Setting registers */
    thread_t* main_thread = p->main_thread;
    thread_set_stack(main_thread, stack_zone->start + VMM_PAGE_SIZE, stack_zone->start + VMM_PAGE_SIZE);
    thread_set_eip(main_thread, code_zone->start);

    kfree(prog);

    return 0;
}

int proc_load(proc_t* p, const char* path)
{
    dentry_t* file;
    file_descriptor_t fd;

    if (vfs_resolve_path_start_from(p->cwd, path, &file) < 0) {
        return -ENOENT;
    }
    if (vfs_open(file, &fd) < 0) {
        dentry_put(file);
        return -ENOENT;
    }

    /* Put it back, since now we have a new cwd */
    if (p->cwd) {
        dentry_put(p->cwd);
    }

    int ret = _proc_load_bin(p, &fd);

    if (!ret) {
        p->cwd = dentry_get_parent(file);
    }

    dentry_put(file);
    vfs_close(&fd);
    return ret;
}

int proc_copy_of(proc_t* new_proc, thread_t* from_thread)
{
    proc_t* from_proc = from_thread->process;
    memcpy((void*)new_proc->main_thread->tf, (void*)from_thread->tf, sizeof(trapframe_t));

    new_proc->cwd = dentry_duplicate(from_proc->cwd);
    new_proc->tty = from_proc->tty;

    /* TODO: change the size in advance */
    for (int i = 0; i < from_proc->zones.size; i++) {
        proc_zone_t* zone_to_copy = (proc_zone_t*)dynamic_array_get(&from_proc->zones, i);
        dynamic_array_push(&new_proc->zones, zone_to_copy);
    }

    if (from_proc->fds) {
        for (int i = 0; i < MAX_OPENED_FILES; i++) {
            if (from_proc->fds[i].dentry) {
                file_descriptor_t* fd = &new_proc->fds[i];
                vfs_open(from_proc->fds[i].dentry, fd);
            }
        }
    }

    return 0;

    // sched_enqueue(new_proc);
}

int proc_free(proc_t* p)
{
    if (p->status == PROC_DEAD || p->status == PROC_INVALID || p->status == PROC_ALIVE || p->pid == 0) {
        return -ESRCH;
    }

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

    dynamic_array_free(&p->zones);

    /* Key parts deletion. After that line you can't work with this process. */
    proc_kill_all_threads(p);
    p->pid = 0;

    if (!p->is_kthread) {
        vmm_free_pdir(p->pdir);
    }

    return 0;
}

int proc_die(proc_t* p)
{
    FOREACH_THREAD(p) {
        thread_die(thread);
    } END_FOREACH
    p->status = PROC_DYING;
    return 0;
}

thread_t* proc_create_thread(proc_t* p)
{
    thread_t* thread = proc_alloc_thread();
    thread_setup(p, thread);
    sched_enqueue(thread);
    return thread;
}

void proc_kill_all_threads(proc_t* p)
{
    FOREACH_THREAD(p) {
        thread_free(thread);
    } END_FOREACH
}

void proc_kill_all_threads_except(proc_t* p, thread_t* gthread)
{
    FOREACH_THREAD(p) {
        if (thread->tid != gthread->tid)
            thread_free(thread);
    } END_FOREACH
}

/**
 * PROC FD FUNCTIONS
 */

int proc_get_fd_id(proc_t* proc, file_descriptor_t* fd)
{
    /* Calculating id with pointers */
    uint32_t start = (uint32_t)proc->fds;
    uint32_t fd_ptr = (uint32_t)fd;
    fd_ptr -= start;
    int fd_res = fd_ptr / sizeof(file_descriptor_t);
    if (!(fd_ptr % sizeof(file_descriptor_t))) {
        return fd_res;
    }
    return -1;
}

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
