/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <mem/vmm.h>
#include <platform/arm32/registers.h>
#include <platform/arm32/target/general/core_setup.h>
#include <security/defs.h>

#define SECURITY_SPECTRE_MITIGATION

int cortex_a15_setup()
{
#ifdef SECURITY_SPECTRE_MITIGATION
    write_actlr(read_actlr() | 1);
#endif
    return 0;
}