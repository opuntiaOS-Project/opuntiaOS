/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_LIBKERN_C_ATTRS_H
#define _KERNEL_LIBKERN_C_ATTRS_H

#ifndef PACKED
#define PACKED __attribute__((packed))
#endif // PACKED

#ifndef MAYBE_UNUSED
#define MAYBE_UNUSED __attribute__((__unused__))
#endif // MAYBE_UNUSED

#ifndef ALWAYS_INLINE
#define ALWAYS_INLINE __attribute__((always_inline)) inline
#endif // ALWAYS_INLINE

#ifndef NORETURN
#define NORETURN __attribute__((noreturn))
#endif // NORETURN

#ifndef WARN_UNUSED_RESULT
#define WARN_UNUSED_RESULT __attribute__((__warn_unused_result__))
#endif // WARN_UNUSED_RESULT

#endif // _KERNEL_LIBKERN_C_ATTRS_H
