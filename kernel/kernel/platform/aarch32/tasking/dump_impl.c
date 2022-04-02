/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libkern/log.h>
#include <mem/vmm.h>
#include <platform/aarch32/tasking/dump_impl.h>
#include <platform/generic/system.h>

extern uintptr_t read_ip();
extern uintptr_t read_bp();

void dump_regs(dump_data_t* dump_data)
{
}

void dump_backtrace(dump_data_t* dump_data, uintptr_t ip, uintptr_t* bp, int is_kernel)
{
    uint32_t id = 1;
    char buf[64];

    do {
        if (vmm_is_kernel_address(ip) && !is_kernel) {
            return;
        }

        snprintf(buf, 64, "[%d] %x : ", id, ip);
        dump_data->writer(buf);
        int index = dump_data->sym_resolver(dump_data->syms, dump_data->symsn, ip);
        dump_data->writer(&dump_data->strs[index]);
        dump_data->writer("\n");

        if (vmm_is_kernel_address((uintptr_t)bp) != is_kernel) {
            return;
        }

        ip = bp[1];
        bp = (uintptr_t*)*bp;
        id++;
    } while (ip != dump_data->entry_point);

    return;
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
    log("at arm backtrace");
    return 0;
    dump_backtrace(dump_data, read_ip(), (uint32_t*)read_bp(), 1);
    return 0;
}

int dump_kernel_impl_from_tf(dump_data_t* dump_data, const char* err_desc, trapframe_t* tf)
{
    if (err_desc) {
        dump_data->writer(err_desc);
    }
    // dump_dump_backtrace(dump_data, read_ip(), (uint32_t*)read_bp(), 1);backtrace(dump_data, tf->eip, (uint32_t*)tf->ebp, 1);
    return 0;
}
