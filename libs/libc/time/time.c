#include <sys/time.h>
#include <sysdep.h>
#include <time.h>

static const int __days_per_month[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

static inline char __is_leap_year(time_t year)
{
    return ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
}

static void time_to_tm(time_t timep, tm_t* res)
{
    res->tm_sec = timep % 60;
    time_t minutes = timep / 60;
    res->tm_min = minutes % 60;
    time_t hours = minutes / 60;
    res->tm_hour = hours % 24;
    time_t days = hours / 24;
    res->tm_wday = (days + 4) % 7;
    time_t year = 1970;
    while (days >= 365 + __is_leap_year(year)) {
        days -= 365 + __is_leap_year(year);
        year++;
    }
    res->tm_year = year - 1900;
    res->tm_yday = days;
    res->tm_mday = 1;
    if (__is_leap_year(year) && days >= 59) {
        days--;
        if (days == 59) {
            res->tm_mday = 2;
        }
    }

    time_t m;
    for (m = 0; m < 11; m++) {
        if (days <= __days_per_month[m]) {
            break;
        }
        days -= __days_per_month[m];
    }

    res->tm_mday += days;
    res->tm_mon = m;
}

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
    static tm_t tm_gmtime_buf;
    return gmtime_r(timep, &tm_gmtime_buf);
}

tm_t* gmtime_r(const time_t* timep, tm_t* result)
{
    if (!result || !timep) {
        return NULL;
    }
    time_to_tm(*timep, result);
    return result;
}

tm_t* localtime(const time_t* timep)
{
    static tm_t tm_localtime_buf;
    return localtime_r(timep, &tm_localtime_buf);
}

tm_t* localtime_r(const time_t* timep, tm_t* result)
{
    // FIXME: Implement time zone
    if (!result || !timep) {
        return NULL;
    }
    time_to_tm(*timep, result);
    return result;
}

time_t mktime(tm_t* tm)
{
    return 0;
}

int clock_gettime(clockid_t clk_id, timespec_t* tp)
{
    int res = DO_SYSCALL_2(SYS_CLOCK_GETTIME, clk_id, tp);
    RETURN_WITH_ERRNO(res, res, -1);
}

// TODO: Implement
int clock_getres(clockid_t clk_id, timespec_t* res) { return -1; }
int clock_settime(clockid_t clk_id, const timespec_t* tp) { return -1; }
