#ifndef _LIBC_BITS_TIME_H
#define _LIBC_BITS_TIME_H

#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

struct timeval {
    time_t tv_sec;
    uint32_t tv_usec;
};
typedef struct timeval timeval_t;

#define DST_NONE 0 /* not on dst */
#define DST_USA 1 /* USA style dst */
#define DST_AUST 2 /* Australian style dst */
#define DST_WET 3 /* Western European dst */
#define DST_MET 4 /* Middle European dst */
#define DST_EET 5 /* Eastern European dst */
#define DST_CAN 6 /* Canada */
struct timezone {
    int tz_minuteswest; /* minutes west of Greenwich */
    int tz_dsttime; /* type of dst correction */
};
typedef struct timezone timezone_t;

struct timespec {
    time_t tv_sec;
    uint32_t tv_nsec;
};
typedef struct timespec timespec_t;

struct tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
};
typedef struct tm tm_t;

typedef enum {
    CLOCK_REALTIME,
    CLOCK_MONOTONIC,
    CLOCK_PROCESS_CPUTIME_ID,
    CLOCK_THREAD_CPUTIME_ID,
} clockid_t;

__END_DECLS

#endif // _LIBC_BITS_TIME_H