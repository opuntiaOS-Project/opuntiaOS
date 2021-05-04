#ifndef _LIBC_TIME_H
#define _LIBC_TIME_H

#include <bits/time.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

time_t time(time_t* timep);
char* asctime(const tm_t* tm);
char* asctime_r(const tm_t* tm, char* buf);

char* ctime(const time_t* timep);
char* ctime_r(const time_t* timep, char* buf);
tm_t* gmtime(const time_t* timep);
tm_t* gmtime_r(const time_t* timep, tm_t* result);

tm_t* localtime(const time_t* timep);
tm_t* localtime_r(const time_t* timep, tm_t* result);

size_t strftime(char* s, size_t max, const char* format, const tm_t* tm);

time_t mktime(tm_t* tm);

int clock_getres(clockid_t clk_id, timespec_t* res);
int clock_gettime(clockid_t clk_id, timespec_t* tp);
int clock_settime(clockid_t clk_id, const timespec_t* tp);

__END_DECLS

#endif // _LIBC_TIME_H
