/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libkern/errno.h>
#include <io/shared_buffer/shared_buffer.h>
#include <io/sockets/local_socket.h>
#include <libkern/log.h>
#include <mem/kmalloc.h>
#include <platform/generic/system.h>
#include <platform/generic/tasking/trapframe.h>
#include <sys_handler.h>
#include <tasking/sched.h>
#include <tasking/tasking.h>
#include <libkern/libkern.h>

#ifdef __i386__
#define sys_id (tf->eax)
#define param1 (tf->ebx)
#define param2 (tf->ecx)
#define param3 (tf->edx)
#define param4 (tf->esi)
#define param5 (tf->edi)
#define return_with_val(val) \
    (tf->eax = val);         \
    return

#elif __arm__
#define sys_id (tf->r[7])
#define param1 (tf->r[0])
#define param2 (tf->r[1])
#define param3 (tf->r[2])
#define param4 (tf->r[3])
#define param5 (tf->r[4])
#define return_with_val(val) \
    (tf->r[0] = val);        \
    return

#endif

/* From Linux 4.14.0 headers. */
/* https://chromium.googlesource.com/chromiumos/docs/+/master/constants/syscalls.md#x86-32_bit */

static const void* syscalls[] = {
    [SYSRESTART] = sys_restart_syscall,
    [SYSEXIT] = sys_exit,
    [SYSFORK] = sys_fork,
    [SYSREAD] = sys_read,
    [SYSWRITE] = sys_write,
    [SYSOPEN] = sys_open,
    [SYSCLOSE] = sys_close,
    [SYSWAITPID] = sys_waitpid,
    [SYSCREAT] = sys_creat,
    [SYSLINK] = sys_none, // sys_link
    [SYSUNLINK] = sys_unlink,
    [SYSEXEC] = sys_exec,
    [SYSCHDIR] = sys_chdir,
    [SYSSIGACTION] = sys_sigaction,
    [SYSSIGRETURN] = sys_sigreturn, // When this is moved, change signal_caller.s for now.
    [SYSRAISE] = sys_raise,
    [SYSLSEEK] = sys_lseek,
    [SYSGETPID] = sys_getpid,
    [SYSKILL] = sys_kill,
    [SYSMKDIR] = sys_mkdir,
    [SYSRMDIR] = sys_rmdir,
    [SYSMMAP] = sys_mmap,
    [SYSMUNMAP] = sys_munmap,
    [SYSSOCKET] = sys_socket,
    [SYSBIND] = sys_bind,
    [SYSCONNECT] = sys_connect,
    [SYSGETDENTS] = sys_getdents,
    [SYSIOCTL] = sys_ioctl,
    [SYSSETPGID] = sys_setpgid,
    [SYSGETPGID] = sys_getpgid,
    [SYSPTHREADCREATE] = sys_create_thread,
    [SYSSLEEP] = sys_sleep,
    [SYSSELECT] = sys_select,
    [SYSFSTAT] = sys_fstat,
    [SYSSCHEDYIELD] = sys_sched_yield,
    [SYSUNAME] = sys_uname,
    [SYS_SHBUF_CREATE] = sys_shbuf_create,
    [SYS_SHBUF_GET] = sys_shbuf_get,
    [SYS_SHBUF_FREE] = sys_shbuf_free,
};

static inline void set_return(trapframe_t* tf, uint32_t val)
{
    param1 = val;
}

#ifdef __i386__
int ksyscall_impl(int id, int a, int b, int c, int d)
{
    system_disable_interrupts();
    trapframe_t* tf;
    trapframe_t tf_on_stack;
    tf = &tf_on_stack;
    sys_id = id;
    param1 = a;
    param2 = b;
    param3 = c;
    sys_handler(tf);
    /* This hack has to be here, when a context switching happens
       during a syscall (e.g. when block occurs). The hack will start
       interrupts again after it has become a running thread. */
    system_enable_interrupts();
    return param1;
}
#elif __arm__
int ksyscall_impl(int id, int a, int b, int c, int d)
{
    int ret;
    asm volatile(
        "mov r7, %1;\
        mov r0, %2;\
        mov r1, %3;\
        mov r2, %4;\
        mov r3, %5;\
        mov r4, %6;\
        swi 1;\
        mov %0, r0;"
        : "=r"(ret)
        : "r"(id), "r"((int)(a)), "r"((int)(b)), "r"((int)(c)), "r"((int)(d)), "r"((int)(0))
        : "memory", "r0", "r1", "r2", "r3", "r4", "r7");
    return ret;
}
#endif

void sys_handler(trapframe_t* tf)
{
    system_disable_interrupts();
    void (*callee)(trapframe_t*) = (void*)syscalls[sys_id];
    callee(tf);
    system_enable_interrupts_only_counter();
}

void sys_restart_syscall(trapframe_t* tf)
{
}

/**
 * FS SYSCALLS
 */

void sys_open(trapframe_t* tf)
{
    proc_t* p = RUNNIG_THREAD->process;
    file_descriptor_t* fd = proc_get_free_fd(p);
    const char* path = (char*)param1;
    char* kpath = 0;
    if (!str_validate_len(path, 128)) {
        return_with_val(-EINVAL);
    }
    int name_len = strlen(path);
    kpath = kmem_bring_to_kernel(path, name_len + 1);
    uint32_t flags = param2;

    mode_t mode = S_IFREG | S_IRWXU | S_IRWXG | S_IRWXO;
    dentry_t* file;

    if (flags & O_CREATE) {
        vfs_create(p->cwd, kpath, name_len, mode);
    }

    if (vfs_resolve_path_start_from(p->cwd, kpath, &file) < 0) {
        return_with_val(-ENOENT);
    }
    int res = vfs_open(file, fd, param2);
    dentry_put(file);
    if (!res) {
        return_with_val(proc_get_fd_id(p, fd));
    } else {
        return_with_val(res);
    }
}

void sys_close(trapframe_t* tf)
{
    file_descriptor_t* fd = proc_get_fd(RUNNIG_THREAD->process, param1);
    if (!fd) {
        return_with_val(-EBADF);
    }
    return_with_val(vfs_close(fd));
}

/* TODO: copying to/from user! */
void sys_read(trapframe_t* tf)
{
    file_descriptor_t* fd = proc_get_fd(RUNNIG_THREAD->process, (int)param1);
    if (!fd) {
        return_with_val(-EBADF);
    }

    init_read_blocker(RUNNIG_THREAD, fd);

    int res = vfs_read(fd, (uint8_t*)param2, (uint32_t)param3);
    return_with_val(res);
}

/* TODO: copying to/from user! */
void sys_write(trapframe_t* tf)
{
    file_descriptor_t* fd = proc_get_fd(RUNNIG_THREAD->process, (int)param1);
    if (!fd) {
        return_with_val(-EBADF);
    }

    init_write_blocker(RUNNIG_THREAD, fd);

    int res = vfs_write(fd, (uint8_t*)param2, (uint32_t)param3);
    return_with_val(res);
}

void sys_lseek(trapframe_t* tf)
{
    file_descriptor_t* fd = proc_get_fd(RUNNIG_THREAD->process, (int)param1);
    if (!fd) {
        return_with_val(-EBADF);
    }

    int whence = param3;

    switch (whence) {
    case SEEK_SET:
        fd->offset = param2;
        break;
    case SEEK_CUR:
        fd->offset += param2;
        break;
    case SEEK_END:
        fd->offset = fd->dentry->inode->size - param2;
        break;
    default:
        return_with_val(-EINVAL);
    }

    if (fd->offset >= fd->dentry->inode->size) {
        return_with_val(-EOVERFLOW);
    }

    return_with_val(0);
}

void sys_unlink(trapframe_t* tf)
{
    proc_t* p = RUNNIG_THREAD->process;
    const char* path = (char*)param1;
    char* kpath = 0;
    if (!str_validate_len(path, 128)) {
        return_with_val(-EINVAL);
    }
    int name_len = strlen(path);
    kpath = kmem_bring_to_kernel(path, name_len + 1);

    dentry_t* file;
    if (vfs_resolve_path_start_from(p->cwd, kpath, &file) < 0) {
        return_with_val(-ENOENT);
    }

    int ret = vfs_unlink(file);

    dentry_put(file);
    kfree(kpath);
    return_with_val(ret);
}

void sys_creat(trapframe_t* tf)
{
    proc_t* p = RUNNIG_THREAD->process;
    const char* path = (char*)param1;
    mode_t mode = param2;
    char* kpath = 0;
    if (!str_validate_len(path, 128)) {
        return_with_val(-EINVAL);
    }
    int name_len = strlen(path);
    kpath = kmem_bring_to_kernel(path, name_len + 1);

    int err = vfs_create(p->cwd, kpath, name_len, mode);
    if (err) {
        return_with_val(err);
    }

    /* Opening file */
    file_descriptor_t* fd = proc_get_free_fd(p);
    dentry_t* file;
    if (vfs_resolve_path_start_from(p->cwd, kpath, &file) < 0) {
        return_with_val(-1);
    }
    err = vfs_open(file, fd, O_WRONLY);
    dentry_put(file);
    if (err) {
        return_with_val(err);
    }
    return_with_val(proc_get_fd_id(p, fd));
}

void sys_fstat(trapframe_t* tf)
{
    file_descriptor_t* fd = proc_get_fd(RUNNIG_THREAD->process, (int)param1);
    fstat_t* stat = (fstat_t*)param2;
    if (!fd) {
        return_with_val(-EBADF);
    }
    if (!stat) {
        return_with_val(-EINVAL);
    }
    int res = vfs_fstat(fd, stat);
    return_with_val(res);
}

void sys_mkdir(trapframe_t* tf)
{
    proc_t* p = RUNNIG_THREAD->process;
    const char* path = (char*)param1;
    char* kpath = 0;
    if (!str_validate_len(path, 128)) {
        return_with_val(-EINVAL);
    }
    int name_len = strlen(path);
    kpath = kmem_bring_to_kernel(path, name_len + 1);

    // dentry_t* dir;
    // if (vfs_resolve_path_start_from(p->cwd, &dir) < 0) {
    //     return_with_val(-ENOENT);
    // }

    mode_t dir_mode = S_IFDIR | S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
    int res = vfs_mkdir(p->cwd, kpath, name_len, dir_mode);
    kfree(kpath);
    return_with_val(res);
}

void sys_rmdir(trapframe_t* tf)
{
    proc_t* p = RUNNIG_THREAD->process;
    const char* path = (char*)param1;
    char* kpath = 0;
    if (!str_validate_len(path, 128)) {
        return_with_val(-EINVAL);
    }
    int name_len = strlen(path);
    kpath = kmem_bring_to_kernel(path, name_len + 1);

    dentry_t* dir;
    if (vfs_resolve_path_start_from(p->cwd, kpath, &dir) < 0) {
        return_with_val(-ENOENT);
    }

    int res = vfs_rmdir(dir);
    dentry_put(dir);
    kfree(kpath);
    return_with_val(res);
}

void sys_chdir(trapframe_t* tf)
{
    /* proc lock */
    const char* path = (char*)param1;
    return_with_val(proc_chdir(RUNNIG_THREAD->process, path));
}

void sys_getdents(trapframe_t* tf)
{
    proc_t* p = RUNNIG_THREAD->process;
    file_descriptor_t* fd = (file_descriptor_t*)proc_get_fd(p, (uint32_t)param1);
    int read = vfs_getdents(fd, (uint8_t*)param2, param3);
    return_with_val(read);
}

void sys_select(trapframe_t* tf)
{
    proc_t* p = RUNNIG_THREAD->process;
    file_descriptor_t* fd;

    int nfds = param1;
    fd_set_t* readfds = (fd_set_t*)param2;
    fd_set_t* writefds = (fd_set_t*)param3;
    fd_set_t* exceptfds = (fd_set_t*)param4;
    timeval_t* timeout = (timeval_t*)param5;
    if (nfds < 0 || nfds > FD_SETSIZE) {
        return_with_val(-EINVAL);
    }

    for (int i = 0; i < nfds; i++) {
        if (FD_ISSET(i, readfds) || FD_ISSET(i, writefds) || FD_ISSET(i, exceptfds)) {
            if (!proc_get_fd(p, i)) {
                return_with_val(-EBADF);
            }
        }
    }

    init_select_blocker(RUNNIG_THREAD, nfds, readfds, writefds, exceptfds, timeout);

    if (readfds)
        FD_ZERO(readfds);
    if (writefds)
        FD_ZERO(writefds);
    if (exceptfds)
        FD_ZERO(exceptfds);

    for (int i = 0; i < nfds; i++) {
        fd = proc_get_fd(p, i);
        if (readfds && FD_ISSET(i, &(RUNNIG_THREAD->readfds))) {
            if (fd->ops->can_read && fd->ops->can_read(fd->dentry, fd->offset)) {
                FD_SET(i, readfds);
            }
        }
        if (writefds && FD_ISSET(i, &(RUNNIG_THREAD->writefds))) {
            if (fd->ops->can_write && fd->ops->can_write(fd->dentry, fd->offset)) {
                FD_SET(i, writefds);
            }
        }
    }

    return_with_val(0);
}

/**
 * TASKING SYSCALLS
 */

void sys_exit(trapframe_t* tf)
{
    tasking_exit((int)param1);
}

void sys_fork(trapframe_t* tf)
{
    tasking_fork(tf);
}

void sys_waitpid(trapframe_t* tf)
{
    int ret = tasking_waitpid(param1);
    set_return(tf, ret);
}

void sys_exec(trapframe_t* tf)
{
    int res = tasking_exec((char*)param1, (const char**)param2, (const char**)param3);
    if (res != 0) {
        set_return(tf, res);
    }
}

void sys_sigaction(trapframe_t* tf)
{
    set_return(tf, signal_set_handler(RUNNIG_THREAD, (int)param1, (void*)param2));
}

void sys_sigreturn(trapframe_t* tf)
{
    signal_restore_thread_after_handling_signal(RUNNIG_THREAD);
}

void sys_raise(trapframe_t* tf)
{
    signal_set_pending(RUNNIG_THREAD, (int)param1);
    signal_dispatch_pending(RUNNIG_THREAD);
}

void sys_getpid(trapframe_t* tf)
{
    return_with_val(RUNNIG_THREAD->tid);
}

void sys_kill(trapframe_t* tf)
{
    thread_t* thread = thread_by_pid(param1);
    return_with_val(tasking_kill(thread, param2));
}

void sys_setpgid(trapframe_t* tf)
{
    uint32_t pid = param1;
    uint32_t pgid = param2;

    proc_t* p = tasking_get_proc(pid);
    if (!p) {
        return_with_val(-ESRCH);
    }

    p->pgid = pgid;
    return_with_val(0);
}

void sys_getpgid(trapframe_t* tf)
{
    uint32_t pid = param1;

    proc_t* p = tasking_get_proc(pid);
    if (!p) {
        return_with_val(-ESRCH);
    }

    return_with_val(p->pgid);
}

void sys_create_thread(trapframe_t* tf)
{
    proc_t* p = RUNNIG_THREAD->process;
    thread_t* thread = proc_create_thread(p);
    if (!thread) {
        return_with_val(-EFAULT);
    }

    thread_create_params_t* params = (thread_create_params_t*)param1;
    set_instruction_pointer(thread->tf, params->entry_point);
    uint32_t esp = params->stack_start + params->stack_size;
    set_stack_pointer(thread->tf, esp);
    set_base_pointer(thread->tf, esp);

    return_with_val(thread->tid);
}

void sys_sleep(trapframe_t* tf)
{
    thread_t* p = RUNNIG_THREAD;
    time_t time = param1;

    init_sleep_blocker(p, time);

    return_with_val(0);
}

/**
 * MEMORY SYSCALLS
 */

void sys_mmap(trapframe_t* tf)
{
    proc_t* p = RUNNIG_THREAD->process;
    mmap_params_t* params = (mmap_params_t*)param1;

    bool map_shared = ((params->flags & MAP_SHARED) > 0);
    bool map_anonymous = ((params->flags & MAP_ANONYMOUS) > 0);
    bool map_private = ((params->flags & MAP_PRIVATE) > 0);
    bool map_stack = ((params->flags & MAP_STACK) > 0);
    bool map_fixed = ((params->flags & MAP_FIXED) > 0);

    bool map_exec = ((params->prot & PROT_EXEC) > 0);
    bool map_read = ((params->prot & PROT_READ) > 0);
    bool map_write = ((params->prot & PROT_WRITE) > 0);

    proc_zone_t* zone;

    if (map_private && map_shared) {
        return_with_val(-EINVAL);
    }
    if (!map_private && !map_shared) {
        return_with_val(-EINVAL);
    }

    if (map_stack) {
        zone = proc_new_random_zone_backward(p, params->size);
    } else if (map_anonymous) {
        zone = proc_new_random_zone(p, params->size);
    } else {
        file_descriptor_t* fd = proc_get_fd(p, params->fd);
        /* TODO: Check for read access to the file */
        if (!fd) {
            return_with_val(-EBADFD);
        }
        zone = vfs_mmap(fd, params);
    }

    if (!zone) {
        return_with_val(-ENOMEM);
    }

    if (map_read) {
        zone->flags |= ZONE_READABLE;
    }
    if (map_write) {
        zone->flags |= ZONE_WRITABLE;
    }
    if (map_exec) {
        zone->flags |= ZONE_EXECUTABLE;
    }

    return_with_val(zone->start);
}

void sys_munmap(trapframe_t* tf)
{
}

/**
 * IO SYSCALLS
 */

void sys_socket(trapframe_t* tf)
{
    proc_t* p = RUNNIG_THREAD->process;
    int domain = param1;
    int type = param2;
    int protocol = param3;

    file_descriptor_t* fd = proc_get_free_fd(p);
    if (!fd) {
        return_with_val(-1);
    }

    if (domain == PF_LOCAL) {
        int res = local_socket_create(type, protocol, fd);
        if (!res) {
            return_with_val(proc_get_fd_id(p, fd));
        } else {
            return_with_val(res);
        }
    }
}

void sys_bind(trapframe_t* tf)
{
    proc_t* p = RUNNIG_THREAD->process;
    int sockfd = param1;
    char* name = (char*)param2;
    uint32_t len = (uint32_t)param3;

    file_descriptor_t* sfd = proc_get_fd(p, sockfd);
    if (sfd->type != FD_TYPE_SOCKET || !sfd->sock_entry) {
        return_with_val(-EBADF);
    }

    if (sfd->sock_entry->domain == PF_LOCAL) {
        return_with_val(local_socket_bind(sfd, name, len));
    }

    return_with_val(0);
}

void sys_connect(trapframe_t* tf)
{
    proc_t* p = RUNNIG_THREAD->process;
    int sockfd = param1;
    char* name = (char*)param2;
    uint32_t len = (uint32_t)param3;

    file_descriptor_t* sfd = proc_get_fd(p, sockfd);
    if (sfd->type != FD_TYPE_SOCKET || !sfd->sock_entry) {
        return_with_val(-EBADF);
    }

    if (sfd->sock_entry->domain == PF_LOCAL) {
        return_with_val(local_socket_connect(sfd, name, len));
    }

    return_with_val(-EFAULT);
}

void sys_ioctl(trapframe_t* tf)
{
    proc_t* p = RUNNIG_THREAD->process;
    file_descriptor_t* fd = proc_get_fd(p, param1);

    if (!fd) {
        return_with_val(-EBADF);
    }

    if (fd->dentry->ops->file.ioctl) {
        return_with_val(fd->dentry->ops->file.ioctl(fd->dentry, param2, param3));
    } else {
        return_with_val(-EACCES);
    }
}

void sys_shbuf_create(trapframe_t* tf)
{
    uint8_t** buffer = (uint8_t**)param1;
    size_t size = param2;
    return_with_val(shared_buffer_create(buffer, size));
}

void sys_shbuf_get(trapframe_t* tf)
{
    int id = param1;
    uint8_t** buffer = (uint8_t**)param2;
    return_with_val(shared_buffer_get(id, buffer));
}

void sys_shbuf_free(trapframe_t* tf)
{
    int id = param1;
    return_with_val(shared_buffer_free(id));
}

/**
 * Scheduler
 */

void sys_sched_yield(trapframe_t* tf)
{
    resched();
}

/**
 * Misc
 */

void sys_uname(trapframe_t* tf)
{
    utsname_t* buf = (utsname_t*)param1;
    vmm_copy_to_user(buf->sysname, "oneOS", 6);
    vmm_copy_to_user(buf->release, "1.0.0-dev", 10);
    vmm_copy_to_user(buf->version, "1", 2);
#ifdef __i386__
    vmm_copy_to_user(buf->machine, "x86", 4);
#elif __arm__
    vmm_copy_to_user(buf->machine, "ARM", 4);
#endif
    return_with_val(0);
}

void sys_none(trapframe_t* tf) { }
