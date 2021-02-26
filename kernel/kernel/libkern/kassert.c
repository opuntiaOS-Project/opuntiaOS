/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libkern/kassert.h>

void kpanic(char* err_msg)
{
    log_error("Kpanic occured %s", err_msg);
    system_stop();
}