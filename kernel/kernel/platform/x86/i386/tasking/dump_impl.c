/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libkern/log.h>
#include <mem/vmm.h>
#include <platform/x86/registers.h>
#include <platform/x86/system.h>
#include <platform/x86/tasking/dump_impl.h>

void dump_regs(dump_data_t* dump_data)
{
    // 3 rows of: 3chars (reg name) + 2chars (separartor) + 12chars (reg value)
    char buf[64];
    trapframe_t* tf = dump_data->p->main_thread->tf;
    snprintf(buf, 64, "EAX: %x  EBX: %x  ECX: %x\n", tf->eax, tf->ebx, tf->ecx);
    dump_data->writer(buf);
    snprintf(buf, 64, "EDX: %x  ESI: %x  EDI: %x\n", tf->edx, tf->esi, tf->edi);
    dump_data->writer(buf);
    snprintf(buf, 64, "EIP: %x  ESP: %x  EBP: %x\n\n", tf->eip, tf->esp, tf->ebp);
    dump_data->writer(buf);
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

        snprintf(buf, 64, "[%d] %x : ", id, ip);
        dump_data->writer(buf);
        int index = dump_data->sym_resolver(dump_data->syms, dump_data->symsn, ip);
        dump_data->writer(&dump_data->strs[index]);
        dump_data->writer("\n");

        if (IS_KERNEL_VADDR((uintptr_t)bp) != is_kernel) {
            return;
        }

        ip = bp[1];
        uintptr_t* newbp = (uintptr_t*)*bp;
        if (bp == newbp) {
            return;
        }
        bp = newbp;
        id++;
    } while (ip != dump_data->entry_point);

    return;
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
    dump_backtrace(dump_data, read_ip(), (uintptr_t*)read_bp(), 1);
    return 0;
}

int dump_kernel_impl_from_tf(dump_data_t* dump_data, const char* err_desc, trapframe_t* tf)
{
    if (err_desc) {
        dump_data->writer(err_desc);
    }
    dump_backtrace(dump_data, tf->eip, (uintptr_t*)tf->ebp, 1);
    return 0;
}
