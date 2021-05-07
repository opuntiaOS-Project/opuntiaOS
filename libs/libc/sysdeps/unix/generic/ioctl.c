#include <sys/ioctl.h>
#include <sysdep.h>

int ioctl(int fd, uint32_t cmd, uint32_t arg)
{
    int res = DO_SYSCALL_3(SYS_IOCTL, fd, cmd, arg);
    RETURN_WITH_ERRNO(res, res, -1);
}