/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#ifndef __oneOS__UTILS__KASSERT_H
#define __oneOS__UTILS__KASSERT_H

#include <log.h>
#include <platform/generic/system.h>
#include <types.h>

#define ASSERT(x)                                              \
    if (unlikely(!(x))) {                                      \
        log("kassert at line %d in %s\n", __LINE__, __FILE__); \
        system_stop();                                         \
    }

void kpanic(char* msg);

#endif // __oneOS__UTILS__KASSERT_H
