/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libkern/kassert.h>
// #include <tasking/dump.h>

void assert_handler(const char* cond, const char* func, const char* file, int line)
{
    log("Kernel assertion failed: %s, function %s, file %s:%d\n", cond, func, file, line);
    while (1) { }
    // dump_kernel(NULL);
    // system_stop();
}

void kpanic(const char* err_msg)
{
    // dump_kernel(err_msg);
    // system_stop();
    while (1) { }
}

void kpanic_tf(const char* err_msg, trapframe_t* tf)
{
    // dump_kernel_from_tf(err_msg, tf);
    // system_stop();
    while (1) { }
}