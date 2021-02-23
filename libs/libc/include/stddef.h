#ifndef __oneOS__LibC__STDDEF_H
#define __oneOS__LibC__STDDEF_H

#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

#ifdef __i386__
typedef unsigned long size_t;
#elif __arm__
typedef unsigned int size_t;
#endif

#define NULL ((void*)0)

__END_DECLS

#endif // __oneOS__LibC__STDDEF_H