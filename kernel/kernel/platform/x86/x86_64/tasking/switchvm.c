/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <mem/vmm.h>
#include <platform/generic/system.h>
#include <platform/x86/fpu/fpu.h>
#include <platform/x86/gdt.h>
#include <platform/x86/tasking/switchvm.h>
#include <platform/x86/tasking/tss.h>

/* switching the page dir and tss to the current proc */
void switchuvm(thread_t* thread)
{
    system_disable_interrupts();

    tss_t* tssptr = &tss;
    uint32_t tssptrlo = (uintptr_t)tssptr & 0xffffffff;
    uint32_t tssptrhi = ((uintptr_t)tssptr) >> 32;

    gdt[GDT_SEG_TSS] = GDT_SEG_TSS_DESC(SEGTSS_TYPE, tssptrlo, sizeof(tss) - 1, 0);
    gdt[GDT_SEG_TSS + 1] = GDT_SEG_SET_RAW(tssptrhi);

    uintptr_t esp0 = ((uintptr_t)thread->tf + sizeof(trapframe_t));
    tss.rsp0l = esp0 & 0xffffffff;
    tss.rsp0h = esp0 >> 32;
    tss.iomapbase = sizeof(tss);
    RUNNING_THREAD = thread;

    fpu_make_unavail();

    set_ltr(GDT_SEG_TSS << 3);

    vmm_switch_address_space(thread->process->address_space);
    system_enable_interrupts();
}