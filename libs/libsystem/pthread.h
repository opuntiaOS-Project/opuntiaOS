#ifndef __oneOS__LibC__PTHREAD_H
#define __oneOS__LibC__PTHREAD_H

#include "types.h"

//TODO: calc based on totalram_pages
#define MAX_THREADS 120

typedef unsigned int pthread_t; // thread identifier

typedef struct {
    
} __pthread_t;

__pthread_t pthreads[MAX_THREADS];

typedef struct {
    bool lock;
} pthread_mutex_t;

void pthread_mutex_lock(pthread_mutex_t*);
void pthread_mutex_unlock(pthread_mutex_t*);

#endif
