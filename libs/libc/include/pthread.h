#ifndef _LIBC_PTHREAD_H
#define _LIBC_PTHREAD_H

#include <sys/_structs.h>
#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

int pthread_create(void* func);

__END_DECLS

#endif /* _LIBC_PTHREAD_H */