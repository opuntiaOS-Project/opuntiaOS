/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libobjc/memory.h>

id alloc_instance(Class cls)
{
    size_t sz = cls->size();
    id obj = (id)objc_malloc(sz);
    obj->set_isa(cls);
    return obj;
}