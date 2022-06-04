#include <sys/time.h>
#include <sysdep.h>

int nanosleep(const timespec_t* req, timespec_t* rem)
{
    int res = DO_SYSCALL_2(SYS_NANOSLEEP, req, rem);
    RETURN_WITH_ERRNO(res, 0, -1);
}

int gettimeofday(timeval_t* tv, timezone_t* tz)
{
    int res = DO_SYSCALL_2(SYS_GETTIMEOFDAY, tv, tz);
    RETURN_WITH_ERRNO(res, res, -1);
}

int settimeofday(const timeval_t* tv, const timezone_t* tz)
{
    return -1;
}