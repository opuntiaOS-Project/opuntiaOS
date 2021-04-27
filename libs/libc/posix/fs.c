#include <fcntl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sysdep.h>
#include <unistd.h>

int open(const char* pathname, int flags)
{
    return DO_SYSCALL_3(SYS_OPEN, pathname, flags, S_IFREG | S_IRWXU | S_IRWXG | S_IRWXO);
}

int creat(const char* path, mode_t mode)
{
    return DO_SYSCALL_2(SYS_CREAT, path, mode);
}

int close(int fd)
{
    return DO_SYSCALL_1(SYS_CLOSE, fd);
}

ssize_t read(int fd, char* buf, size_t count)
{
    return (ssize_t)DO_SYSCALL_3(SYS_READ, fd, buf, count);
}

ssize_t write(int fd, const void* buf, size_t count)
{
    return (ssize_t)DO_SYSCALL_3(SYS_WRITE, fd, buf, count);
}

off_t lseek(int fd, off_t off, int whence)
{
    return (off_t)DO_SYSCALL_3(SYS_LSEEK, fd, off, whence);
}

int mkdir(const char* path)
{
    return DO_SYSCALL_1(SYS_MKDIR, path);
}

int rmdir(const char* path)
{
    return DO_SYSCALL_1(SYS_RMDIR, path);
}

int chdir(const char* path)
{
    return DO_SYSCALL_1(SYS_CHDIR, path);
}

int unlink(const char* path)
{
    return DO_SYSCALL_1(SYS_UNLINK, path);
}

int fstat(int nfds, fstat_t* stat)
{
    return DO_SYSCALL_2(SYS_FSTAT, nfds, stat);
}

int select(int nfds, fd_set_t* readfds, fd_set_t* writefds, fd_set_t* exceptfds, timeval_t* timeout)
{
    return DO_SYSCALL_5(SYS_SELECT, nfds, readfds, writefds, exceptfds, timeout);
}

void* mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    mmap_params_t mmap_params = { 0 };
    mmap_params.addr = addr;
    mmap_params.size = length;
    mmap_params.prot = prot;
    mmap_params.flags = flags;
    mmap_params.fd = fd;
    mmap_params.offset = offset;
    return (void*)DO_SYSCALL_1(SYS_MMAP, &mmap_params);
}

int munmap(void* addr, size_t length)
{
    return DO_SYSCALL_2(SYS_MUNMAP, addr, length);
}