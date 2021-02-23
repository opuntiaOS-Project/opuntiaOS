#ifndef __oneOS__LibC__ERRNO_H
#define __oneOS__LibC__ERRNO_H

#include "../../kernel/include/errno.h"

extern int errno;

#define set_errno(x) (errno = x)

#endif