/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <tasking/cpu.h>

cpu_t cpus[MAX_CPU_CNT];

void _asm_cpu_enter_user_space()
{
    cpu_enter_user_space();
}
