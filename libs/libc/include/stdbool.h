#ifndef _LIBC_STDBOOL_H
#define _LIBC_STDBOOL_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#ifndef __bool_true_false_are_defined
#define bool _Bool
#define true (1)
#define false (0)
#define __bool_true_false_are_defined 1
#endif // __bool_true_false_are_defined

__END_DECLS

#endif //_LIBC_STDBOOL_H