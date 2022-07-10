/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libkern/log.h>
#include <mem/vmm.h>
#include <platform/arm64/tasking/dump_impl.h>
#include <platform/generic/system.h>

static void dump_xregs_row(dump_data_t* dump_data, int a)
{
    char buf[64];
    trapframe_t* tf = dump_data->p->main_thread->tf;
    snprintf(buf, 64, "x%d: %zx  x%d: %zx  x%d: %zx\n", a, tf->x[a], a + 1, tf->x[a + 1], a + 2, tf->x[a + 2]);
    dump_data->writer(buf);
}

void dump_regs(dump_data_t* dump_data)
{
    trapframe_t* tf = dump_data->p->main_thread->tf;
    for (int i = 0; i < 30; i += 3) {
        dump_xregs_row(dump_data, i);
    }
}

void dump_backtrace(dump_data_t* dump_data, uintptr_t ip, uintptr_t* bp, int is_kernel)
{
    size_t id = 1;
    char buf[64];

    do {
        if (id > 64) {
            return;
        }
        if (IS_KERNEL_VADDR(ip) && !is_kernel) {
            return;
        }
        if (ip == 0) {
            return;
        }

        snprintf(buf, 64, "[%zd] %zx : ", id, ip);
        dump_data->writer(buf);
        int index = dump_data->sym_resolver(dump_data->syms, dump_data->symsn, ip);
        dump_data->writer(&dump_data->strs[index]);
        dump_data->writer("\n");

        if (IS_KERNEL_VADDR((uintptr_t)bp) != is_kernel) {
            return;
        }

        // TODO: This might be not always correct for arm64.
        ip = bp[1];
        uintptr_t* newbp = (uintptr_t*)*bp;
        if (bp == newbp) {
            return;
        }
        bp = newbp;
        id++;
    } while (ip != dump_data->entry_point);
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
    dump_backtrace(dump_data, read_ip(), (uintptr_t*)read_fp(), 1);
    return 0;
}

int dump_kernel_impl_from_tf(dump_data_t* dump_data, const char* err_desc, trapframe_t* tf)
{
    if (err_desc) {
        dump_data->writer(err_desc);
    }
    dump_backtrace(dump_data, tf->elr, (uintptr_t*)tf->x[30], 1);
    return 0;
}
