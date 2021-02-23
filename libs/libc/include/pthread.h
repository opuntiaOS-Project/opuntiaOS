#ifndef __oneOS__LibC__PTHREAD_H
#define __oneOS__LibC__PTHREAD_H

#include <sys/_structs.h>
#include <sys/types.h>
#include <sys/cdefs.h>

__BEGIN_DECLS

int pthread_create(void* func);

__END_DECLS

#endif /* __oneOS__LibC__PTHREAD_H */