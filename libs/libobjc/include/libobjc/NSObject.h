/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _LIBOBJC_NSOBJECT_H
#define _LIBOBJC_NSOBJECT_H

#include <libobjc/runtime.h>

@interface NSObject {
    Class isa;
}

+ (id)init;
+ (id)new;
+ (id)alloc;

- (id)init;
- (Class)class;

@end

#endif // _LIBOBJC_NSOBJECT_H