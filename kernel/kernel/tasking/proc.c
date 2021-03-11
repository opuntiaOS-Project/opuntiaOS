/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <fs/vfs.h>
#include <io/tty/tty.h>
#include <libkern/bits/errno.h>
#include <libkern/libkern.h>
#include <libkern/log.h>
#include <libkern/syscall_structs.h>
#include <mem/kmalloc.h>
#include <tasking/elf.h>
#include <tasking/proc.h>
#include <tasking/sched.h>
#include <tasking/tasking.h>
#include <tasking/thread.h>

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

thread_t* thread_by_pid(uint32_t pid)
{
    for (int i = 0; i < threads_cnt; i++) {
        if (thread_storage[i].tid == pid) {
            return &thread_storage[i];
        }
    }
    return 0;
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

    /* setting dentries */
    p->proc_file = NULL;
    p->cwd = NULL;

    /* allocating space for open files */
    p->fds = kmalloc(MAX_OPENED_FILES * sizeof(file_descriptor_t));
    if (!p->fds) {
        return -ENOMEM;
    }
    memset((void*)p->fds, 0, MAX_OPENED_FILES * sizeof(file_descriptor_t));

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

    char* path_to_tty = "/dev/tty ";
    path_to_tty[8] = tty->id + '0';
    dentry_t* tty_dentry;
    if (vfs_resolve_path(path_to_tty, &tty_dentry) < 0) {
        return -ENOENT;
    }
    int err = vfs_open(tty_dentry, fd0, O_RDWR);
    if (err) {
        return err;
    }
    err = vfs_open(tty_dentry, fd1, O_RDWR);
    if (err) {
        return err;
    }
    dentry_put(tty_dentry);
    return 0;
}

int proc_copy_of(proc_t* new_proc, thread_t* from_thread)
{
    proc_t* from_proc = from_thread->process;
    memcpy((uint8_t*)new_proc->main_thread->tf, (uint8_t*)from_thread->tf, sizeof(trapframe_t));
#ifdef FPU_ENABLED
    memcpy((uint8_t*)new_proc->main_thread->fpu_state, (uint8_t*)from_thread->fpu_state, sizeof(fpu_state_t));
#endif

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
                if (from_proc->fds[i].type == FD_TYPE_FILE) {
                    vfs_open(from_proc->fds[i].dentry, fd, from_proc->fds[i].flags);
                }
            }
        }
    }

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

    proc_zone_t* bss_zone = proc_new_random_zone(p, 2 * 4096);
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
    set_base_pointer(main_thread->tf, stack_zone->start + VMM_PAGE_SIZE);
    set_stack_pointer(main_thread->tf, stack_zone->start + VMM_PAGE_SIZE);
    set_instruction_pointer(main_thread->tf, code_zone->start);

    kfree(prog);
    return 0;
}

int proc_load(proc_t* p, thread_t* main_thread, const char* path)
{
    file_descriptor_t fd;
    dentry_t* dentry;

    if (vfs_resolve_path_start_from(p->cwd, path, &dentry) < 0) {
        return -ENOENT;
    }
    if (vfs_open(dentry, &fd, O_RDONLY) < 0) {
        dentry_put(dentry);
        return -ENOENT;
    }

    // Saving data to restore in case of error.
    pdirectory_t* old_pdir = p->pdir;
    dynamic_array_t old_zones = p->zones;

    // Reallocating proc.
    pdirectory_t* new_pdir = vmm_new_user_pdir();
    vmm_switch_pdir(new_pdir);
    p->pdir = new_pdir;

    if (dynamic_array_init_of_size(&p->zones, sizeof(proc_zone_t), 8) != 0) {
        dentry_put(dentry);
        vfs_close(&fd);
        return -ENOMEM;
    }

    int err = elf_load(p, &fd);
    if (err) {
        goto restore;
    }

success:
    p->main_thread = main_thread;

    // Clearing proc
    proc_kill_all_threads_except(p, p->main_thread);
    p->pid = p->main_thread->tid;
    if (p->proc_file) {
        dentry_put(p->proc_file);
    }
#ifdef FPU_ENABLED
    fpu_reset_state(p->main_thread->fpu_state);
#endif
    dynamic_array_clear(&old_zones);
    vmm_free_pdir(old_pdir);

    // Setting up proc
    p->proc_file = dentry; // dentry isn't put, but is transfered to the proc.
    if (!p->cwd) {
        p->cwd = dentry_get_parent(p->proc_file);
    }
    vfs_close(&fd);
    return 0;

restore:
    dynamic_array_clear(&p->zones);
    p->zones = old_zones;
    p->pdir = old_pdir;
    vmm_switch_pdir(old_pdir);
    vmm_free_pdir(new_pdir);
    vfs_close(&fd);
    dentry_put(dentry);
    return err;
}

/**
 * PROC FREE FUNCTIONS
 */

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

    if (p->proc_file) {
        dentry_put(p->proc_file);
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
        p->pdir = 0;
    }

    return 0;
}

int proc_die(proc_t* p)
{
    FOREACH_THREAD(p)
    {
        thread_die(thread);
    }
    END_FOREACH
    p->status = PROC_DYING;
    return 0;
}

int proc_block_all_threads(proc_t* p, blocker_t* blocker)
{
    FOREACH_THREAD(p)
    {
        thread->status = THREAD_BLOCKED;
        thread->blocker.reason = blocker->reason;
        thread->blocker.should_unblock = blocker->should_unblock;
        thread->blocker.should_unblock_for_signal = blocker->should_unblock_for_signal;
        sched_dequeue(thread);
    }
    END_FOREACH
    return 0;
}

/**
 * PROC THREAD FUNCTIONS
 */

thread_t* proc_create_thread(proc_t* p)
{
    thread_t* thread = proc_alloc_thread();
    thread_setup(p, thread);
    sched_enqueue(thread);
    return thread;
}

void proc_kill_all_threads(proc_t* p)
{
    FOREACH_THREAD(p)
    {
        thread_free(thread);
    }
    END_FOREACH
}

void proc_kill_all_threads_except(proc_t* p, thread_t* gthread)
{
    FOREACH_THREAD(p)
    {
        if (thread->tid != gthread->tid)
            thread_free(thread);
    }
    END_FOREACH
}

/**
 * PROC FS FUNCTIONS
 */

int proc_chdir(proc_t* p, const char* path)
{
    char* kpath = NULL;
    if (!str_validate_len(path, 128)) {
        return -EINVAL;
    }
    kpath = kmem_bring_to_kernel(path, strlen(path) + 1);

    dentry_t* new_cwd = NULL;
    int ret = vfs_resolve_path_start_from(p->cwd, kpath, &new_cwd);
    if (ret) {
        return -ENOENT;
    }

    if (!dentry_inode_test_flag(new_cwd, S_IFDIR)) {
        dentry_put(new_cwd);
        return -ENOTDIR;
    }

    /* Put an old one back */
    if (p->cwd) {
        dentry_put(p->cwd);
    }
    p->cwd = new_cwd;
    return 0;
}

int proc_get_fd_id(proc_t* p, file_descriptor_t* fd)
{
    ASSERT(p->fds);
    /* Calculating id with pointers */
    uint32_t start = (uint32_t)p->fds;
    uint32_t fd_ptr = (uint32_t)fd;
    fd_ptr -= start;
    int fd_res = fd_ptr / sizeof(file_descriptor_t);
    if (!(fd_ptr % sizeof(file_descriptor_t))) {
        return fd_res;
    }
    return -1;
}

file_descriptor_t* proc_get_free_fd(proc_t* p)
{
    ASSERT(p->fds);

    for (int i = 0; i < MAX_OPENED_FILES; i++) {
        if (!p->fds[i].dentry) {
            return &p->fds[i];
        }
    }

    return NULL;
}

file_descriptor_t* proc_get_fd(proc_t* p, uint32_t index)
{
    ASSERT(p->fds);

    if (index >= MAX_OPENED_FILES) {
        return NULL;
    }

    if (!p->fds[index].dentry) {
        return NULL;
    }

    return &p->fds[index];
}
