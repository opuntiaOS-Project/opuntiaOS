/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <fs/vfs.h>
#include <io/sockets/socket.h>
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

static pid_t proc_next_pid = 1;
thread_list_t thread_list;
int threads_cnt = 0;

/**
 * LOCKLESS
 */

static void proc_kill_all_threads_except_locked(proc_t* p, thread_t* gthread);
static ALWAYS_INLINE void proc_kill_all_threads_locked(proc_t* p);
static int proc_setup_locked(proc_t* p);
static int proc_setup_vconsole_locked(proc_t* p, vconsole_entry_t* vconsole);

static int proc_load_locked(proc_t* p, thread_t* main_thread, const char* path);
static int proc_chdir_locked(proc_t* p, const char* path);

static file_descriptor_t* proc_get_free_fd_locked(proc_t* p);
static file_descriptor_t* proc_get_fd_locked(proc_t* p, size_t index);
static ALWAYS_INLINE int fd_is_opened(file_descriptor_t* fd);

/**
 * THREAD STORAGE
 */

static thread_list_node_t* proc_alloc_thread_storage_node()
{
    thread_list_node_t* res = (thread_list_node_t*)kmalloc(sizeof(thread_list_node_t));
    memset(res->thread_storage, 0, sizeof(res->thread_storage));
    res->empty_spots = THREADS_PER_NODE;
    res->next = NULL;
    return res;
}

static thread_t* _proc_alloc_thread()
{
    ASSERT(thread_list.next_empty_node != NULL);
    spinlock_acquire(&thread_list.lock);
    if (!thread_list.next_empty_node->empty_spots) {
        thread_list_node_t* node = proc_alloc_thread_storage_node();
        thread_list.tail->next = node;
        thread_list.tail = node;
        thread_list.next_empty_node = node;
        thread_list.next_empty_index = 0;
    }

    for (int i = 0; i < THREADS_PER_NODE; i++) {
        if (thread_is_freed(&thread_list.next_empty_node->thread_storage[i])) {
            thread_list.next_empty_node->empty_spots--;
            thread_list.next_empty_index++;
            thread_list.next_empty_node->thread_storage[i].status = THREAD_STATUS_ALLOCATED;
            spinlock_release(&thread_list.lock);
            return &thread_list.next_empty_node->thread_storage[i];
        }
    }

    ASSERT(false);
}

/**
 * HELPER FUNCTIONS
 */

#define foreach_thread(p)                                                                                                              \
    for (thread_list_node_t* __thread_list_node = thread_list.head; __thread_list_node; __thread_list_node = __thread_list_node->next) \
        for (int __i = 0; __i < THREADS_PER_NODE; __i++)                                                                               \
            for (thread_t* thread = &__thread_list_node->thread_storage[__i]; thread && thread_is_alive(thread); thread = NULL)        \
                if (thread->process->pid == p->pid)

thread_t* proc_alloc_thread()
{
    return _proc_alloc_thread();
}

thread_t* thread_by_pid(pid_t pid)
{
    thread_list_node_t* __thread_list_node = thread_list.head;
    while (__thread_list_node) {
        for (int i = 0; i < THREADS_PER_NODE; i++) {
            if (__thread_list_node->thread_storage[i].process->pid == pid) {
                return &__thread_list_node->thread_storage[i];
            }
        }
        __thread_list_node = __thread_list_node->next;
    }

    return NULL;
}

pid_t proc_alloc_pid()
{
    return atomic_add(&proc_next_pid, 1);
}

int proc_init_storage()
{
    spinlock_init(&thread_list.lock);
    thread_list_node_t* node = proc_alloc_thread_storage_node();
    thread_list.head = node;
    thread_list.tail = node;
    thread_list.next_empty_node = node;
    thread_list.next_empty_index = 0;
    return 0;
}

/**
 * INIT FUNCTIONS
 */

static int proc_setup_locked(proc_t* p)
{
    p->pid = proc_alloc_pid();
    p->pgid = p->pid;
    p->ppid = 0;
    p->uid = 0;
    p->gid = 0;
    p->euid = 0;
    p->egid = 0;
    p->suid = 0;
    p->sgid = 0;
    p->is_kthread = false;

    p->main_thread = proc_alloc_thread();
    int res = thread_setup_main(p, p->main_thread);
    if (res != 0) {
        return res;
    }

    /* setting dentries */
    p->proc_file = NULL;
    p->cwd = vfs_empty_path();

    /* allocating space for open files */
    p->fds = kmalloc(MAX_OPENED_FILES * sizeof(file_descriptor_t));
    if (!p->fds) {
        return -ENOMEM;
    }
    memset((void*)p->fds, 0, MAX_OPENED_FILES * sizeof(file_descriptor_t));

    p->status = PROC_ALIVE;
    p->prio = DEFAULT_PRIO;
    return 0;
}

int proc_setup(proc_t* p)
{
    spinlock_acquire(&p->lock);
    int res = proc_setup_locked(p);
    spinlock_release(&p->lock);
    return res;
}

int proc_setup_with_uid(proc_t* p, uid_t uid, gid_t gid)
{
    spinlock_acquire(&p->lock);
    int err = proc_setup_locked(p);
    p->uid = uid;
    p->gid = gid;
    p->euid = uid;
    p->egid = gid;
    p->suid = uid;
    p->sgid = gid;
    spinlock_release(&p->lock);
    return err;
}

static int proc_setup_vconsole_locked(proc_t* p, vconsole_entry_t* vconsole)
{
    file_descriptor_t* fd0 = &p->fds[0];
    file_descriptor_t* fd1 = &p->fds[1];
    file_descriptor_t* fd2 = &p->fds[2];

    char* path_to_tty = "/dev/tty ";
    path_to_tty[8] = vconsole->id + '0';
    path_t tty_path;
    if (vfs_resolve_path(path_to_tty, &tty_path) < 0) {
        return -ENOENT;
    }
    int err = vfs_open(&tty_path, fd0, O_RDWR);
    if (err) {
        return err;
    }
    err = vfs_open(&tty_path, fd1, O_RDWR);
    if (err) {
        return err;
    }
    err = vfs_open(&tty_path, fd2, O_RDWR);
    if (err) {
        return err;
    }
    path_put(&tty_path);
    return 0;
}

int proc_setup_vconsole(proc_t* p, vconsole_entry_t* vconsole)
{
    spinlock_acquire(&p->lock);
    int res = proc_setup_vconsole_locked(p, vconsole);
    spinlock_release(&p->lock);
    return res;
}

int proc_fork_from(proc_t* new_proc, thread_t* from_thread)
{
    proc_t* from_proc = from_thread->process;
    new_proc->address_space = vmm_new_forked_address_space();

    thread_copy_of(new_proc->main_thread, from_thread);

    new_proc->ppid = from_proc->pid;
    new_proc->pgid = from_proc->gid;
    new_proc->uid = from_proc->uid;
    new_proc->gid = from_proc->gid;
    new_proc->euid = from_proc->euid;
    new_proc->egid = from_proc->egid;
    new_proc->suid = from_proc->suid;
    new_proc->sgid = from_proc->sgid;
    new_proc->cwd = path_duplicate(&from_proc->cwd);
    new_proc->proc_file = file_duplicate(from_proc->proc_file);

    if (from_proc->fds) {
        for (int i = 0; i < MAX_OPENED_FILES; i++) {
            if (fd_is_opened(&from_proc->fds[i])) {
                file_descriptor_t* fd = &new_proc->fds[i];
                proc_copy_fd(&from_proc->fds[i], fd);
            }
        }
    }

    return 0;
}

/**
 * LOAD FUNCTIONS
 */

static int proc_load_locked(proc_t* p, thread_t* main_thread, const char* path)
{
    path_t bin_path;
    if (vfs_resolve_path_start_from(&p->cwd, path, &bin_path) != 0) {
        return -ENOENT;
    }

    file_descriptor_t fd;
    if (vfs_open(&bin_path, &fd, O_EXEC) != 0) {
        path_put(&bin_path);
        return -ENOENT;
    }

    // Saving data to restore in case of error.
    vm_address_space_t* old_aspace = p->address_space;

    // Reallocating proc.
    vm_address_space_t* new_aspace = vmm_new_address_space();
    p->address_space = new_aspace;
    vmm_switch_address_space(p->address_space);

    p->main_thread = main_thread;
    int err = elf_load(p, &fd);
    if (err) {
        goto restore;
    }

success:
    // Clearing proc
    proc_kill_all_threads_except_locked(p, p->main_thread);
    p->pid = p->main_thread->tid;
    if (p->proc_file) {
        file_put(p->proc_file);
    }
#ifdef FPU_ENABLED
    fpu_init_state(p->main_thread->fpu_state);
#endif

    if (old_aspace) {
        vm_address_space_free(old_aspace);
    }

    // Setting up proc
    p->proc_file = file_init_path(&bin_path);
    if (!path_is_valid(&p->cwd)) {
        p->cwd.dentry = dentry_duplicate(dentry_get_parent(bin_path.dentry));
    }

    if (TEST_FLAG(bin_path.dentry->inode->mode, S_ISUID)) {
        p->euid = bin_path.dentry->inode->uid;
        p->suid = bin_path.dentry->inode->uid;
    }

    if (TEST_FLAG(bin_path.dentry->inode->mode, S_ISGID)) {
        p->egid = bin_path.dentry->inode->gid;
        p->sgid = bin_path.dentry->inode->gid;
    }

    path_put(&bin_path);
    vfs_close(&fd);
    return 0;

restore:
    p->address_space = old_aspace;
    vmm_switch_address_space(p->address_space);
    vm_address_space_free(new_aspace);
    vfs_close(&fd);
    path_put(&bin_path);
    return err;
}

int proc_load(proc_t* p, thread_t* main_thread, const char* path)
{
    spinlock_acquire(&p->vm_lock);
    spinlock_acquire(&p->lock);
    int res = proc_load_locked(p, main_thread, path);
    spinlock_release(&p->lock);
    spinlock_release(&p->vm_lock);
    return res;
}

/**
 * PROC FREE FUNCTIONS
 */

int proc_free_locked(proc_t* p)
{
    if (p->status != PROC_DYING || p->pid == 0) {
        return -ESRCH;
    }

    if (p->fds) {
        for (int i = 0; i < MAX_OPENED_FILES; i++) {
            if (fd_is_opened(&p->fds[i])) {
                vfs_close(&p->fds[i]);
            }
        }
        kfree(p->fds);
    }

    if (p->proc_file) {
        file_put(p->proc_file);
    }
    if (path_is_valid(&p->cwd)) {
        path_put(&p->cwd);
    }

    // Key parts deletion. After that line you can't work with this process.
    proc_kill_all_threads_locked(p);

    if (!p->is_kthread) {
        vm_address_space_free(p->address_space);
        p->address_space = NULL;
    }

    p->is_tracee = false;
    return 0;
}

int proc_free(proc_t* p)
{
    spinlock_acquire(&p->vm_lock);
    spinlock_acquire(&p->lock);
    int res = proc_free_locked(p);
    spinlock_release(&p->lock);
    spinlock_release(&p->vm_lock);
    return res;
}

int proc_die(proc_t* p)
{
    spinlock_acquire(&p->lock);
    p->status = PROC_DYING;
    foreach_thread(p)
    {
        thread_die(thread);
    }

    tasking_evict_zombies_waiting_for(p);
    spinlock_release(&p->lock);
    return 0;
}

int proc_block_all_threads(proc_t* p, const blocker_t* blocker)
{
    spinlock_acquire(&p->lock);
    foreach_thread(p)
    {
        thread_init_blocker(thread, blocker);
    }
    spinlock_release(&p->lock);
    return 0;
}

/**
 * PROC THREAD FUNCTIONS
 */

thread_t* proc_create_thread(proc_t* p)
{
    spinlock_acquire(&p->lock);
    thread_t* thread = proc_alloc_thread();
    thread_setup(p, thread);
    sched_enqueue(thread);
    spinlock_release(&p->lock);
    return thread;
}

static void proc_kill_all_threads_except_locked(proc_t* p, thread_t* gthread)
{
    foreach_thread(p)
    {
        if (!gthread || thread->tid != gthread->tid) {
            thread_free(thread);
        }
    }
}

static ALWAYS_INLINE void proc_kill_all_threads_locked(proc_t* p)
{
    proc_kill_all_threads_except_locked(p, NULL);
}

void proc_kill_all_threads_except(proc_t* p, thread_t* gthread)
{
    spinlock_acquire(&p->lock);
    proc_kill_all_threads_except_locked(p, gthread);
    spinlock_release(&p->lock);
}

void proc_kill_all_threads(proc_t* p)
{
    proc_kill_all_threads_except(p, NULL);
}

/**
 * PROC FS FUNCTIONS
 */

static int proc_chdir_locked(proc_t* p, const char* path)
{
    path_t new_cwd_path;
    int ret = vfs_resolve_path_start_from(&p->cwd, path, &new_cwd_path);
    if (ret) {
        return -ENOENT;
    }

    if (!dentry_test_mode(new_cwd_path.dentry, S_IFDIR)) {
        path_put(&new_cwd_path);
        return -ENOTDIR;
    }

    // Put an old one back.
    if (path_is_valid(&p->cwd)) {
        path_put(&p->cwd);
    }
    p->cwd = new_cwd_path;
    return 0;
}

int proc_chdir(proc_t* p, const char* path)
{
    spinlock_acquire(&p->lock);
    int res = proc_chdir_locked(p, path);
    spinlock_release(&p->lock);
    return res;
}

int proc_get_fd_id(proc_t* p, file_descriptor_t* fd)
{
    spinlock_acquire(&p->lock);
    ASSERT(p->fds);
    /* Calculating id with pointers */
    uintptr_t start = (uintptr_t)p->fds;
    uintptr_t fd_ptr = (uintptr_t)fd;
    fd_ptr -= start;
    int fd_res = fd_ptr / sizeof(file_descriptor_t);
    if (!(fd_ptr % sizeof(file_descriptor_t))) {
        spinlock_release(&p->lock);
        return fd_res;
    }
    spinlock_release(&p->lock);
    return -1;
}

static ALWAYS_INLINE int fd_is_opened(file_descriptor_t* fd)
{
    return fd->file != NULL;
}

static file_descriptor_t* proc_get_free_fd_locked(proc_t* p)
{
    ASSERT(p->fds);

    for (int i = 0; i < MAX_OPENED_FILES; i++) {
        if (!fd_is_opened(&p->fds[i])) {
            return &p->fds[i];
        }
    }

    return NULL;
}

file_descriptor_t* proc_get_free_fd(proc_t* p)
{
    spinlock_acquire(&p->lock);
    file_descriptor_t* res = proc_get_free_fd_locked(p);
    spinlock_release(&p->lock);
    return res;
}

static file_descriptor_t* proc_get_fd_locked(proc_t* p, size_t index)
{
    ASSERT(p->fds);

    if (index >= MAX_OPENED_FILES) {
        return NULL;
    }

    if (!fd_is_opened(&p->fds[index])) {
        return NULL;
    }

    return &p->fds[index];
}

file_descriptor_t* proc_get_fd(proc_t* p, size_t index)
{
    spinlock_acquire(&p->lock);
    file_descriptor_t* res = proc_get_fd_locked(p, index);
    spinlock_release(&p->lock);
    return res;
}

int proc_copy_fd(file_descriptor_t* oldfd, file_descriptor_t* newfd)
{
    newfd->file = file_duplicate(oldfd->file);
    newfd->offset = oldfd->offset;
    newfd->flags = oldfd->flags;
    return 0;
}
