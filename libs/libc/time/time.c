#include <sys/time.h>
#include <sysdep.h>
#include <time.h>

time_t time(time_t* timep)
{
    timeval_t tv;
    timezone_t tz;
    if (gettimeofday(&tv, &tz) != 0) {
        return (time_t)-1;
    }
    if (timep) {
        *timep = tv.tv_sec;
    }
    return tv.tv_sec;
}

char* asctime(const tm_t* tm)
{
    return NULL;
}

char* asctime_r(const tm_t* tm, char* buf)
{
    return NULL;
}

char* ctime(const time_t* timep)
{
    return NULL;
}

char* ctime_r(const time_t* timep, char* buf)
{
    return NULL;
}

tm_t* gmtime(const time_t* timep)
{
    time_t tp = *timep;

    return NULL;
}

tm_t* gmtime_r(const time_t* timep, tm_t* result)
{
    return NULL;
}

tm_t* localtime(const time_t* timep)
{
    return NULL;
}

tm_t* localtime_r(const time_t* timep, tm_t* result)
{
    return NULL;
}

time_t mktime(tm_t* tm)
{
    return 0;
}

int clock_gettime(clockid_t clk_id, timespec_t* tp)
{
    return DO_SYSCALL_2(SYS_CLOCK_GETTIME, clk_id, tp);
}

// TODO: Implement
int clock_getres(clockid_t clk_id, timespec_t* res) { return -1; }
int clock_settime(clockid_t clk_id, const timespec_t* tp) { return -1; }
