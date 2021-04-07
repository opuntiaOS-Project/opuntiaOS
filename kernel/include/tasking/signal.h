/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_TASKING_SIGNAL_H
#define _KERNEL_TASKING_SIGNAL_H

#include <libkern/types.h>

#define SIGNALS_CNT 32

/* TODO: Add more */
enum SIGNAL_ACTION {
    SIGNAL_ACTION_TERMINATE,
    SIGNAL_ACTION_STOP,
    SIGNAL_ACTION_CONTINUE,
    SIGINT,

};

#endif // _KERNEL_TASKING_SIGNAL_H