/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _LIBOBJC_RUNTIME_H
#define _LIBOBJC_RUNTIME_H

#include <libobjc/helpers.h>
#include <libobjc/isa.h>
#include <libobjc/selector.h>
#include <libobjc/v1/decls.h>

#define ROOT_CLASS "NSObject"

OBJC_EXPORT void objc_msgSend(id reciever, SEL sel, ...);

static inline Class object_getClass(id object)
{
    if (!object) {
        return Nil;
    }
    return object->get_isa();
}

#endif // _LIBOBJC_RUNTIME_H