/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _LIBOBJC_OBJC_H
#define _LIBOBJC_OBJC_H

#include <libobjc/helpers.h>
#include <libobjc/runtime.h>

OBJC_EXPORT id _Nullable objc_alloc(Class _Nullable cls);
OBJC_EXPORT id _Nullable objc_alloc_init(Class _Nullable cls);

#endif // _LIBOBJC_OBJC_H