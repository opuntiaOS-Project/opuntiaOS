#ifndef _LIBOBJC_RUNTIME_H
#define _LIBOBJC_RUNTIME_H

#include <libobjc/isa.h>
#include <libobjc/selector.h>
#include <libobjc/v1/decls.h>

static inline Class object_getClass(id object)
{
    if (!object) {
        return Nil;
    }
    return object->get_isa();
}

#endif // _LIBOBJC_RUNTIME_H