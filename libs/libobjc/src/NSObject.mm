/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libobjc/NSObject.h>
#include <libobjc/class.h>
#include <libobjc/memory.h>
#include <libobjc/objc.h>
#include <libobjc/runtime.h>

OBJC_EXPORT IMP objc_msg_lookup(id receiver, SEL sel)
{
    IMP impl = class_get_implementation(receiver->get_isa(), sel);
    return impl;
}

static inline id call_alloc(Class cls, bool checkNil, bool allocWithZone = false)
{
    if (allocWithZone) {
        return ((id(*)(id, SEL, void*))objc_msgSend)(cls, @selector(allocWithZone:), NULL);
    }
    return ((id(*)(id, SEL))objc_msgSend)(cls, @selector(alloc));
}

// Called with [Class alloc]
OBJC_EXPORT id objc_alloc(Class cls)
{
    return call_alloc(cls, true, false);
}

// Called with [[Class alloc] init]
OBJC_EXPORT id objc_alloc_init(Class cls)
{
    return [call_alloc(cls, true, false) init];
}

@implementation NSObject

+ (id)init
{
    return (id)self;
}

- (id)init
{
    // Init root classes
    return (id)self;
}

+ (id)alloc
{
    return alloc_instance(self);
}

+ (id)new
{
    return [call_alloc(self, false) init];
}

- (Class)class
{
    return object_getClass(self);
}

@end