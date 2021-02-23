/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __oneOS__LIBCXX__UTILS__ASSERT_H
#define __oneOS__LIBCXX__UTILS__ASSERT_H

#include <std/Dbg.h>

#define ASSERT(x)                                                          \
    if (!(x)) {                                                            \
        Dbg() << "assert at line" << __LINE__ << " in " __FILE__ << ".\n"; \
        exit(1);                                                           \
    }

#define ASSERT_NOT_REACHED() ASSERT(false)

#endif // __oneOS__LIBCXX__UTILS__ASSERT_H
