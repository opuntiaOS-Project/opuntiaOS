#ifndef _LIBC_ASSERT_H
#define _LIBC_ASSERT_H

#include <stddef.h>
#include <stdio.h>
#include <sys/_structs.h>
#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

#ifndef assert
#define assert(x)                                              \
    if (!(x)) {                                                \
        printf("assert at line %d in %s", __LINE__, __FILE__); \
        abort();                                               \
    }
#endif // assert

__END_DECLS

#endif // _LIBC_ASSERT_H