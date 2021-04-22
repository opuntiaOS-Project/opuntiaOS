#include <sysdep.h>
#include <time.h>

int clock_gettime(clockid_t clk_id, timespec_t* tp)
{
    return DO_SYSCALL_2(SYS_CLOCK_GETTIME, clk_id, tp);
}

// TODO: Implement
int clock_getres(clockid_t clk_id, timespec_t* res) { return -1; }
int clock_settime(clockid_t clk_id, const timespec_t* tp) { return -1; }
