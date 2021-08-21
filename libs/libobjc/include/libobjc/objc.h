#ifndef _LIBOBJC_OBJC_H
#define _LIBOBJC_OBJC_H

#include <libobjc/helpers.h>
#include <libobjc/runtime.h>

OBJC_EXPORT id _Nullable objc_alloc(Class _Nullable cls);
OBJC_EXPORT id _Nullable objc_alloc_init(Class _Nullable cls);

#endif // _LIBOBJC_OBJC_H