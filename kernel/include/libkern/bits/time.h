#ifndef _KERNEL_LIBKERN_BITS_TIME_H
#define _KERNEL_LIBKERN_BITS_TIME_H

#include <libkern/types.h>

struct timeval {
    time_t tv_sec;
    uint32_t tv_usec;
};
typedef struct timeval timeval_t;

struct timespec {
    time_t tv_sec;
    uint32_t tv_nsec;
};
typedef struct timespec timespec_t;

typedef enum {
    CLOCK_REALTIME,
    CLOCK_MONOTONIC,
    CLOCK_PROCESS_CPUTIME_ID,
    CLOCK_THREAD_CPUTIME_ID,
} clockid_t;

#endif // _KERNEL_LIBKERN_BITS_TIME_H
