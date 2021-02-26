#ifndef _LIBC_SYS_TIME_H
#define _LIBC_SYS_TIME_H

#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

typedef uint32_t time_t;

struct timeval {
    time_t tv_sec;
    uint32_t tv_usec;
};
typedef struct timeval timeval_t;

__END_DECLS

#endif