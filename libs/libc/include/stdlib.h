#ifndef _LIBC_STDLIB_H
#define _LIBC_STDLIB_H

#include <stddef.h>
#include <sys/cdefs.h>
#include <sys/environ.h>
#include <sys/types.h>

__BEGIN_DECLS

#ifndef NOMINMAX
#ifndef max
#define max(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
#endif /* max */

#ifndef min
#define min(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })
#endif /* min */
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

/* tools */
int atoi(const char* s);

/* exit */
void abort() __attribute__((noreturn));
void exit(int status) __attribute__((noreturn));

/* pts */
int posix_openpt(int flags);
int ptsname_r(int fd, char* buf, size_t buflen);
char* ptsname(int fd);

/* env */
char* getenv(const char* name);

__END_DECLS

#endif // _LIBC_STDLIB_H
