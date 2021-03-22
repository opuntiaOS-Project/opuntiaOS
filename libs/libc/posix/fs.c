#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sysdep.h>
#include <unistd.h>

int open(const char* pathname, int flags)
{
    return DO_SYSCALL_2(SYSOPEN, (int)pathname, flags);
}

int creat(const char* path, mode_t mode)
{
    return DO_SYSCALL_2(SYSCREAT, (int)path, (int)mode);
}

ssize_t read(int fd, char* buf, size_t count)
{
    return (ssize_t)DO_SYSCALL_3(SYSREAD, (int)fd, (int)buf, (int)count);
}

ssize_t write(int fd, const void* buf, size_t count)
{
    return (ssize_t)DO_SYSCALL_3(SYSWRITE, (int)fd, (int)buf, (int)count);
}

int mkdir(const char* path)
{
    return DO_SYSCALL_1(SYSMKDIR, (int)path);
}

int rmdir(const char* path)
{
    return DO_SYSCALL_1(SYSRMDIR, (int)path);
}

int chdir(const char* path)
{
    return DO_SYSCALL_1(SYSCHDIR, (int)path);
}

int unlink(const char* path)
{
    return DO_SYSCALL_1(SYSUNLINK, (int)path);
}

int fstat(int nfds, fstat_t* stat)
{
    return DO_SYSCALL_2(SYSFSTAT, (int)nfds, (int)stat);
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
    return (void*)DO_SYSCALL_1(SYSMMAP, &mmap_params);
}

int munmap(void* addr, size_t length)
{
    return DO_SYSCALL_2(SYSMUNMAP, addr, length);
}