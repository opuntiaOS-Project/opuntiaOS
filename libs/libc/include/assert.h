#ifndef _LIBC_ASSERT_H
#define _LIBC_ASSERT_H

#include <stddef.h>
#include <stdio.h>
#include <sys/_structs.h>
#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

#ifndef assert
#define assert(x)                                                                                    \
    if (!(x)) {                                                                                      \
        printf("Assertion failed: %s, function %s, file %s:%d\n", #x, __func__, __FILE__, __LINE__); \
        fflush(stdout);                                                                              \
        abort();                                                                                     \
    }
#endif // assert

__END_DECLS

#endif // _LIBC_ASSERT_H