#include <sys/ioctl.h>
#include <sysdep.h>

int ioctl(int fd, uint32_t cmd, uint32_t arg)
{
    return DO_SYSCALL_3(SYSIOCTL, fd, cmd, arg);
}