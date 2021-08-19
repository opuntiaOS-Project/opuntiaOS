#ifndef _LIBOBJC_OBJC_H
#define _LIBOBJC_OBJC_H

#include <libobjc/helpers.h>

struct objc_class;
struct objc_object;
#define nil (struct objc_object*)0
#define Nil (struct objc_class*)0

OBJC_EXPORT id _Nullable objc_alloc(Class _Nullable cls);
OBJC_EXPORT id _Nullable objc_alloc_init(Class _Nullable cls);

#endif // _LIBOBJC_OBJC_H