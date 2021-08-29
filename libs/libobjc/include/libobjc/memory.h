/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _LIBOBJC_MEMORY_H
#define _LIBOBJC_MEMORY_H

#include <libobjc/runtime.h>
#include <stdlib.h>

#define objc_malloc(...) malloc(__VA_ARGS__)
#define objc_realloc(...) realloc(__VA_ARGS__)
#define objc_calloc(...) calloc(__VA_ARGS__)
#define objc_free(...) free(__VA_ARGS__)

id alloc_instance(Class cls);

#endif // _LIBOBJC_MEMORY_H