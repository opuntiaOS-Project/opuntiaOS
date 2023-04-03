/*
 * Copyright (C) 2020-2023 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libkern/log.h>
#include <mem/vmm.h>
#include <platform/arm64/tasking/dump_impl.h>
#include <platform/generic/system.h>

static const char* reg_names[] = {
    "ra",
    "sp",
    "gp",
    "tp",
    "t0",
    "t1",
    "t2",
    "s0",
    "s1",
    "a0",
    "a1",
    "a2",
    "a3",
    "a4",
    "a5",
    "a6",
    "a7",
    "s2",
    "s3",
    "s4",
    "s5",
    "s6",
    "s7",
    "s8",
    "s9",
    "s10",
    "s11",
    "t3",
    "t4",
    "t5",
    "t6",
    "-",
    "-"
};

static void dump_xregs_row(dump_data_t* dump_data, int a)
{
    char buf[64];
    uint64_t* rawtf = &(dump_data->p->main_thread->tf->ra);
    snprintf(buf, 64, "%s: %zx  %s: %zx  %s: %zx\n", reg_names[a], rawtf[a], reg_names[a + 1], rawtf[a + 1], reg_names[a + 2], rawtf[a + 2]);
    dump_data->writer(buf);
}

void dump_regs(dump_data_t* dump_data)
{
    for (int i = 0; i < 31; i += 3) {
        dump_xregs_row(dump_data, i);
    }
}

void dump_backtrace(dump_data_t* dump_data, uintptr_t ip, uintptr_t* bp, int is_kernel)
{
}

int dump_impl(dump_data_t* dump_data)
{
    trapframe_t* tf = dump_data->p->main_thread->tf;
    dump_regs(dump_data);
    dump_backtrace(dump_data, get_instruction_pointer(tf), (uintptr_t*)get_frame_pointer(tf), 0);
    return 0;
}

int dump_kernel_impl(dump_data_t* dump_data, const char* err_desc)
{
    if (err_desc) {
        dump_data->writer(err_desc);
    }
    dump_backtrace(dump_data, read_ip(), (uintptr_t*)NULL, 1);
    return 0;
}

int dump_kernel_impl_from_tf(dump_data_t* dump_data, const char* err_desc, trapframe_t* tf)
{
    if (err_desc) {
        dump_data->writer(err_desc);
    }
    dump_backtrace(dump_data, tf->epc, (uintptr_t*)NULL, 1);
    return 0;
}
