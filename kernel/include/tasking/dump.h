/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_TASKING_DUMP_H
#define _KERNEL_TASKING_DUMP_H

#include <tasking/proc.h>

void dump_and_kill(proc_t* p);

#endif // _KERNEL_TASKING_DUMP_H