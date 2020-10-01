#ifndef __oneOS__LibC__SYS__TIME_H
#define __oneOS__LibC__SYS__TIME_H

#include <sys/types.h>

typedef uint32_t time_t;

struct timeval {
    time_t tv_sec;
    uint32_t tv_usec;
};
typedef struct timeval timeval_t;

#endif