#ifndef _LIBOBJC_HELPERS_H
#define _LIBOBJC_HELPERS_H

#include <stdio.h>

#define OBJC_EXPORT extern "C"

#define OBJC_DEBUG

#ifdef OBJC_DEBUG
#define OBJC_DEBUGPRINT(...) \
    printf(__VA_ARGS__);     \
    fflush(stdout)
#else
#define OBJC_DEBUGPRINT(...) (sizeof(int))
#endif

#endif // _LIBOBJC_HELPERS_H