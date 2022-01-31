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

static ALWAYS_INLINE void proc_kill_all_threads_except_lockless(proc_t* p, thread_t* gthread);
static ALWAYS_INLINE void proc_kill_all_threads_lockless(proc_t* p);
static ALWAYS_INLINE int proc_setup_lockless(proc_t* p);
static ALWAYS_INLINE int proc_setup_tty_lockless(proc_t* p, tty_entry_t* tty);

static ALWAYS_INLINE int proc_load_lockless(proc_t* p, thread_t* main_thread, const char* path);
static ALWAYS_INLINE int proc_chdir_lockless(proc_t* p, const char* path);

static ALWAYS_INLINE file_descriptor_t* proc_get_free_fd_lockless(proc_t* p);
static ALWAYS_INLINE file_descriptor_t* proc_get_fd_lockless(proc_t* p, uint32_t index);

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
    lock_acquire(&thread_list.lock);
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
            lock_release(&thread_list.lock);
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
    lock_init(&thread_list.lock);
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

static ALWAYS_INLINE int proc_setup_lockless(proc_t* p)
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
    p->cwd = NULL;

    /* allocating space for open files */
    p->fds = kmalloc(MAX_OPENED_FILES * sizeof(file_descriptor_t));
    if (!p->fds) {
        return -ENOMEM;
    }
    memset((void*)p->fds, 0, MAX_OPENED_FILES * sizeof(file_descriptor_t));

    /* setting up zones */
    if (dynarr_init_of_size(memzone_t, &p->zones, 8) != 0) {
        return -ENOMEM;
    }

    p->status = PROC_ALIVE;
    p->prio = DEFAULT_PRIO;
    return 0;
}

int proc_setup(proc_t* p)
{
    lock_acquire(&p->lock);
    int res = proc_setup_lockless(p);
    lock_release(&p->lock);
    return res;
}

int proc_setup_with_uid(proc_t* p, uid_t uid, gid_t gid)
{
    lock_acquire(&p->lock);
    int err = proc_setup_lockless(p);
    p->uid = uid;
    p->gid = gid;
    p->euid = uid;
    p->egid = gid;
    p->suid = uid;
    p->sgid = gid;
    lock_release(&p->lock);
    return err;
}

static ALWAYS_INLINE int proc_setup_tty_lockless(proc_t* p, tty_entry_t* tty)
{
    file_descriptor_t* fd0 = &p->fds[0];
    file_descriptor_t* fd1 = &p->fds[1];
    file_descriptor_t* fd2 = &p->fds[2];
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
    err = vfs_open(tty_dentry, fd2, O_RDWR);
    if (err) {
        return err;
    }
    dentry_put(tty_dentry);
    return 0;
}

int proc_setup_tty(proc_t* p, tty_entry_t* tty)
{
    lock_acquire(&p->lock);
    int res = proc_setup_tty_lockless(p, tty);
    lock_release(&p->lock);
    return res;
}

int proc_fork_from(proc_t* new_proc, thread_t* from_thread)
{
    proc_t* from_proc = from_thread->process;
    thread_copy_of(new_proc->main_thread, from_thread);

    new_proc->ppid = from_proc->pid;
    new_proc->pgid = from_proc->gid;
    new_proc->uid = from_proc->uid;
    new_proc->gid = from_proc->gid;
    new_proc->euid = from_proc->euid;
    new_proc->egid = from_proc->egid;
    new_proc->suid = from_proc->suid;
    new_proc->sgid = from_proc->sgid;
    new_proc->cwd = dentry_duplicate(from_proc->cwd);
    new_proc->proc_file = dentry_duplicate(from_proc->proc_file);
    new_proc->tty = from_proc->tty;

    if (from_proc->fds) {
        for (int i = 0; i < MAX_OPENED_FILES; i++) {
            if (from_proc->fds[i].dentry) {
                file_descriptor_t* fd = &new_proc->fds[i];
                if (from_proc->fds[i].type == FD_TYPE_FILE) {
                    vfs_open(from_proc->fds[i].dentry, fd, from_proc->fds[i].flags);
                }
                if (from_proc->fds[i].type == FD_TYPE_SOCKET) {
                    fd->type = FD_TYPE_SOCKET;
                    fd->sock_entry = socket_duplicate(from_proc->fds[i].sock_entry);
                    fd->offset = from_proc->fds[i].offset;
                    fd->flags = from_proc->fds[i].flags;
                    fd->ops = from_proc->fds[i].ops;
                    lock_init(&fd->lock);
                }
            }
        }
    }

    for (int i = 0; i < from_proc->zones.size; i++) {
        memzone_t* zone_to_copy = (memzone_t*)dynarr_get(&from_proc->zones, i);
        if (zone_to_copy->file) {
            dentry_duplicate(zone_to_copy->file); // For the copied zone.
        }
        dynarr_push(&new_proc->zones, zone_to_copy);
    }

    return 0;
}

/**
 * LOAD FUNCTIONS
 */

static int _proc_load_bin(proc_t* p, file_descriptor_t* fd)
{
    uint32_t code_size = fd->dentry->inode->size;
    memzone_t* code_zone = memzone_new_random(p, code_size);
    code_zone->type = ZONE_TYPE_CODE;
    code_zone->flags |= ZONE_READABLE | ZONE_EXECUTABLE;

    /* THIS IS FOR BSS WHICH COULD BE IN THIS ZONE */
    code_zone->flags |= ZONE_WRITABLE;

    memzone_t* bss_zone = memzone_new_random(p, 2 * 4096);
    bss_zone->type = ZONE_TYPE_DATA;
    bss_zone->flags |= ZONE_READABLE | ZONE_WRITABLE;

    memzone_t* stack_zone = memzone_new_random_backward(p, VMM_PAGE_SIZE);
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

static ALWAYS_INLINE int proc_load_lockless(proc_t* p, thread_t* main_thread, const char* path)
{
    int err;
    file_descriptor_t fd;
    dentry_t* dentry;

    if (vfs_resolve_path_start_from(p->cwd, path, &dentry) != 0) {
        return -ENOENT;
    }
    if (vfs_open(dentry, &fd, O_EXEC) != 0) {
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

    if (dynarr_init_of_size(memzone_t, &p->zones, 8) != 0) {
        dentry_put(dentry);
        vfs_close(&fd);
        return -ENOMEM;
    }

    p->main_thread = main_thread;
    err = elf_load(p, &fd);
    if (err) {
        goto restore;
    }

success:
    // Clearing proc
    proc_kill_all_threads_except_lockless(p, p->main_thread);
    p->pid = p->main_thread->tid;
    if (p->proc_file) {
        dentry_put(p->proc_file);
    }
#ifdef FPU_ENABLED
    fpu_init_state(p->main_thread->fpu_state);
#endif

    if (old_pdir) {
        vmm_free_pdir(old_pdir, &old_zones);
    }
    dynarr_clear(&old_zones);

    // Setting up proc
    p->proc_file = dentry; // dentry isn't put, but is transfered to the proc.
    if (!p->cwd) {
        p->cwd = dentry_get_parent(p->proc_file);
    }

    if (TEST_FLAG(fd.dentry->inode->mode, S_ISUID)) {
        p->euid = fd.dentry->inode->uid;
        p->suid = fd.dentry->inode->uid;
    }

    if (TEST_FLAG(fd.dentry->inode->mode, S_ISGID)) {
        p->egid = fd.dentry->inode->gid;
        p->sgid = fd.dentry->inode->gid;
    }

    vfs_close(&fd);
    return 0;

restore:
    p->pdir = old_pdir;
    vmm_switch_pdir(old_pdir);
    vmm_free_pdir(new_pdir, &p->zones);
    dynarr_clear(&p->zones);
    p->zones = old_zones;
    vfs_close(&fd);
    dentry_put(dentry);
    return err;
}

int proc_load(proc_t* p, thread_t* main_thread, const char* path)
{
    lock_acquire(&p->vm_lock);
    lock_acquire(&p->lock);
    int res = proc_load_lockless(p, main_thread, path);
    lock_release(&p->lock);
    lock_release(&p->vm_lock);
    return res;
}

/**
 * PROC FREE FUNCTIONS
 */

int proc_free_lockless(proc_t* p)
{
    if (p->status != PROC_DYING || p->pid == 0) {
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

    /* Key parts deletion. After that line you can't work with this process. */
    proc_kill_all_threads_lockless(p);

    if (!p->is_kthread) {
        vmm_free_pdir(p->pdir, &p->zones);
        p->pdir = NULL;
    }

    p->is_tracee = false;
    dynarr_free(&p->zones);
    return 0;
}

int proc_free(proc_t* p)
{
    lock_acquire(&p->vm_lock);
    lock_acquire(&p->lock);
    int res = proc_free_lockless(p);
    lock_release(&p->lock);
    lock_release(&p->vm_lock);
    return res;
}

int proc_die(proc_t* p)
{
    lock_acquire(&p->lock);
    p->status = PROC_DYING;
    foreach_thread(p)
    {
        thread_die(thread);
    }

    tasking_evict_zombies_waiting_for(p);
    lock_release(&p->lock);
    return 0;
}

int proc_block_all_threads(proc_t* p, const blocker_t* blocker)
{
    lock_acquire(&p->lock);
    foreach_thread(p)
    {
        thread_init_blocker(thread, blocker);
    }
    lock_release(&p->lock);
    return 0;
}

/**
 * PROC THREAD FUNCTIONS
 */

thread_t* proc_create_thread(proc_t* p)
{
    lock_acquire(&p->lock);
    thread_t* thread = proc_alloc_thread();
    thread_setup(p, thread);
    sched_enqueue(thread);
    lock_release(&p->lock);
    return thread;
}

static ALWAYS_INLINE void proc_kill_all_threads_except_lockless(proc_t* p, thread_t* gthread)
{
    foreach_thread(p)
    {
        if (!gthread || thread->tid != gthread->tid) {
            thread_free(thread);
        }
    }
}

static ALWAYS_INLINE void proc_kill_all_threads_lockless(proc_t* p)
{
    proc_kill_all_threads_except_lockless(p, NULL);
}

void proc_kill_all_threads_except(proc_t* p, thread_t* gthread)
{
    lock_acquire(&p->lock);
    proc_kill_all_threads_except_lockless(p, gthread);
    lock_release(&p->lock);
}

void proc_kill_all_threads(proc_t* p)
{
    proc_kill_all_threads_except(p, NULL);
}

/**
 * PROC FS FUNCTIONS
 */

static ALWAYS_INLINE int proc_chdir_lockless(proc_t* p, const char* path)
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

int proc_chdir(proc_t* p, const char* path)
{
    lock_acquire(&p->lock);
    int res = proc_chdir_lockless(p, path);
    lock_release(&p->lock);
    return res;
}

int proc_get_fd_id(proc_t* p, file_descriptor_t* fd)
{
    lock_acquire(&p->lock);
    ASSERT(p->fds);
    /* Calculating id with pointers */
    uintptr_t start = (uintptr_t)p->fds;
    uintptr_t fd_ptr = (uintptr_t)fd;
    fd_ptr -= start;
    int fd_res = fd_ptr / sizeof(file_descriptor_t);
    if (!(fd_ptr % sizeof(file_descriptor_t))) {
        lock_release(&p->lock);
        return fd_res;
    }
    lock_release(&p->lock);
    return -1;
}

static ALWAYS_INLINE file_descriptor_t* proc_get_free_fd_lockless(proc_t* p)
{
    ASSERT(p->fds);

    for (int i = 0; i < MAX_OPENED_FILES; i++) {
        if (!p->fds[i].dentry) {
            return &p->fds[i];
        }
    }

    return NULL;
}

file_descriptor_t* proc_get_free_fd(proc_t* p)
{
    lock_acquire(&p->lock);
    file_descriptor_t* res = proc_get_free_fd_lockless(p);
    lock_release(&p->lock);
    return res;
}

static ALWAYS_INLINE file_descriptor_t* proc_get_fd_lockless(proc_t* p, uint32_t index)
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

file_descriptor_t* proc_get_fd(proc_t* p, uint32_t index)
{
    lock_acquire(&p->lock);
    file_descriptor_t* res = proc_get_fd_lockless(p, index);
    lock_release(&p->lock);
    return res;
}
