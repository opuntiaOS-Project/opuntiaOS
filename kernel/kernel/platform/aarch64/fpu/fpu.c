/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libkern/libkern.h>
#include <platform/aarch64/fpu/fpu.h>
#include <platform/aarch64/registers.h>

void fpu_install()
{
    fpu_enable();
    fpu_make_unavail();
}

void fpu_init_state(fpu_state_t* new_fpu_state)
{
    memset(new_fpu_state, 0, sizeof(fpu_state_t));
}