/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_SECURITY_DEFS_H
#define _KERNEL_SECURITY_DEFS_H

#include <libkern/c_attrs.h>
#include <libkern/types.h>

#define SECURITY_FEATURES

#ifdef SECURITY_FEATURES
#define SECURITY_SPECTRE_MITIGATION
#endif // SECURITY_FEATURES

#endif // _KERNEL_SECURITY_DEFS_H