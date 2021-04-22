#ifndef _LIBC_TIME_H
#define _LIBC_TIME_H

#include <bits/time.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

int clock_getres(clockid_t clk_id, timespec_t* res);
int clock_gettime(clockid_t clk_id, timespec_t* tp);
int clock_settime(clockid_t clk_id, const timespec_t* tp);

__END_DECLS

#endif // _LIBC_TIME_H
