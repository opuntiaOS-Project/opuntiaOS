#include <sys/time.h>
#include <sysdep.h>

int gettimeofday(timeval_t* tv, timezone_t* tz)
{
    int res = DO_SYSCALL_2(SYS_GET_TIME_OF_DAY, tv, tz);
    RETURN_WITH_ERRNO(res, res, -1);
}

int settimeofday(const timeval_t* tv, const timezone_t* tz)
{
    return -1;
}