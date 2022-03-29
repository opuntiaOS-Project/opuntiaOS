/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libkern/log.h>
#include <mem/vmm.h>
#include <platform/generic/system.h>
#include <platform/x86/tasking/dump_impl.h>

void dump_regs(dump_data_t* dump_data)
{
}

void dump_backtrace(dump_data_t* dump_data)
{
}

int dump_impl(dump_data_t* dump_data)
{
    char buf[64];
    trapframe_t* tf = dump_data->p->main_thread->tf;
    snprintf(buf, 64, "Dump not supported for arm target yet!\n");
    dump_data->writer(buf);
    return 0;
}

int dump_kernel_impl(dump_data_t* dump_data, const char* err_desc)
{
    if (err_desc) {
        dump_data->writer(err_desc);
    }
    return 0;
}

int dump_kernel_impl_from_tf(dump_data_t* dump_data, const char* err_desc, trapframe_t* tf)
{
    if (err_desc) {
        dump_data->writer(err_desc);
    }
    return 0;
}
