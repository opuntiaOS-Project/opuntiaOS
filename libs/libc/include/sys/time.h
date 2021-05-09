#ifndef _LIBC_SYS_TIME_H
#define _LIBC_SYS_TIME_H

#include <bits/time.h>
#include <sys/cdefs.h>

__BEGIN_DECLS

int gettimeofday(timeval_t* tv, timezone_t* tz);
int settimeofday(const timeval_t* tv, const timezone_t* tz);

__END_DECLS

#endif