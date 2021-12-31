/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _LIBOBJC_SELECTOR_H
#define _LIBOBJC_SELECTOR_H

#include <libobjc/v1/decls.h>
#include <stddef.h>

static inline bool sel_equal(SEL s1, SEL s2)
{
    if (s1 == NULL || s2 == NULL) {
        return s1 == s2;
    }
    return s1->id == s2->id;
}

void selector_table_init();
void selector_add_from_module(struct objc_selector*);
void selector_add_from_method_list(struct objc_method_list*);
void selector_add_from_class(Class);
bool selector_is_valid(SEL sel);
SEL sel_registerName(const char* name);
SEL sel_registerTypedName(const char* name, const char* types);

#endif // _LIBOBJC_SELECTOR_H