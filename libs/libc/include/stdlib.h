#ifndef _LIBC_STDLIB_H
#define _LIBC_STDLIB_H

#include <stddef.h>
#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

#ifndef NOMINMAX
#define max(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define min(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })
#endif /* NOMINMAX */


static inline int abs(int i)
{
    return i < 0 ? -i : i;
}

/* malloc */
extern void* malloc(size_t);
extern void free(void*);
extern void* calloc(size_t, size_t);
extern void* realloc(void*, size_t);

/* exit */
void exit(int status);

/* pts */
int posix_openpt(int flags);
int ptsname_r(int fd, char* buf, size_t buflen);
char* ptsname(int fd);

__END_DECLS

#endif // _LIBC_STDLIB_H
