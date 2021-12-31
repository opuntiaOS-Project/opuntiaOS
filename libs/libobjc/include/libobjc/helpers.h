/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _LIBOBJC_HELPERS_H
#define _LIBOBJC_HELPERS_H

#include <stdio.h>

#define OBJC_EXPORT extern "C"

#define OBJC_DEBUG

#ifdef OBJC_DEBUG
#define OBJC_DEBUGPRINT(...) \
    printf(__VA_ARGS__);     \
    fflush(stdout)
#else
#define OBJC_DEBUGPRINT(...) (sizeof(int))
#endif

#endif // _LIBOBJC_HELPERS_H