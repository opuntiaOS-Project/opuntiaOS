/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
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
