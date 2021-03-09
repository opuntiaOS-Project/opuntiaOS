#ifndef _LIBC_BITS_THREAD_H
#define _LIBC_BITS_THREAD_H

#include <sys/types.h>

struct thread_create_params {
    uint32_t entry_point;
    uint32_t stack_start;
    uint32_t stack_size;
};
typedef struct thread_create_params thread_create_params_t;

#endif // _LIBC_BITS_THREAD_H