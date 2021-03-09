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

__END_DECLS

#endif // _LIBC_BITS_TIME_H