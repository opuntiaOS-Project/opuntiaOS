/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libobjc/memory.h>
#include <libobjc/objc.h>
#include <libobjc/runtime.h>
#include <libobjc/selector.h>
#include <string.h>

static struct objc_selector* selector_pool_start;
static struct objc_selector* selector_pool_next;

#define CONST_DATA true
#define VOLATILE_DATA false

static SEL selector_table_add(const char* name, const char* types, bool const_data)
{
    // Checking if we have this selector
    for (struct objc_selector* cur_sel = selector_pool_start; cur_sel != selector_pool_next; cur_sel++) {
        if (strcmp(name, (char*)cur_sel->id) == 0) {
            if (cur_sel->types == 0 || types == 0) {
                if (cur_sel->types == types) {
                    return (SEL)cur_sel;
                }
            } else {
                if (strcmp(types, cur_sel->types) == 0) {
                    return (SEL)cur_sel;
                }
            }
        }
    }

    SEL sel = (SEL)selector_pool_next++;
    if (const_data) {
        sel->id = (char*)name;
        sel->types = types;
    } else {
        int name_len = strlen(name);
        char* name_data = (char*)objc_malloc(name_len + 1);
        memcpy(name_data, name, name_len);
        name_data[name_len] = '\0';
        sel->id = name_data;
        sel->types = 0;

        if (types) {
            int types_len = strlen(types);
            char* types_data = (char*)objc_malloc(types_len + 1);
            memcpy(types_data, types, types_len);
            types_data[types_len] = '\0';
            sel->types = types_data;
        }
    }

    return sel;
}

bool selector_is_valid(SEL sel)
{
    return (uintptr_t)selector_pool_start < (uintptr_t)sel && (uintptr_t)sel < (uintptr_t)selector_pool_next;
}

// TODO: We currently do it really stupid
void selector_table_init()
{
    selector_pool_start = selector_pool_next = (struct objc_selector*)malloc(1024);
}

void selector_add_from_module(struct objc_selector* selectors)
{
    for (int i = 0; selectors[i].id; i++) {
        char* name = (char*)selectors[i].id;
        const char* types = selectors[i].types;
        SEL sel = selector_table_add(name, types, CONST_DATA);
    }
}

void selector_add_from_method_list(struct objc_method_list* method_list)
{
    for (int i = 0; i < method_list->method_count; i++) {
        Method method = &method_list->method_list[i];
        if (method->method_name) {
            char* name = (char*)method->method_name;
            const char* types = method->method_types;
            method->method_name = selector_table_add(name, types, CONST_DATA);
        }
    }
}

void selector_add_from_class(Class cls)
{
    for (struct objc_method_list* ml = cls->methods; ml; ml = ml->method_next) {
        selector_add_from_method_list(ml);
    }
}

SEL sel_registerName(const char* name)
{
    if (!name) {
        return (SEL)NULL;
    }

    return selector_table_add(name, 0, VOLATILE_DATA);
}

SEL sel_registerTypedName(const char* name, const char* types)
{
    if (!name || !types) {
        return (SEL)NULL;
    }

    return selector_table_add(name, types, VOLATILE_DATA);
}