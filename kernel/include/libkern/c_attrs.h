/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_LIBKERN_C_ATTRS_H
#define _KERNEL_LIBKERN_C_ATTRS_H

#ifndef PACKED
#define PACKED __attribute__((packed))
#endif // PACKED

#ifndef ALWAYS_INLINE
#define ALWAYS_INLINE __attribute__((always_inline)) inline
#endif // ALWAYS_INLINE

#ifndef NORETURN
#define NORETURN __attribute__((noreturn))
#endif // NORETURN

#endif // _KERNEL_LIBKERN_C_ATTRS_H
