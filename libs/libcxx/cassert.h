/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#ifndef __oneOS__LIBCXX__UTILS__ASSERT_H
#define __oneOS__LIBCXX__UTILS__ASSERT_H

#include <std/Dbg.h>

#define ASSERT(x)                                                          \
    if (!(x)) {                                                            \
        Dbg() << "assert at line" << __LINE__ << " in " __FILE__ << ".\n"; \
        exit(1);                                                          \
    }

#define ASSERT_NOT_REACHED() ASSERT(false)

#endif // __oneOS__LIBCXX__UTILS__ASSERT_H
