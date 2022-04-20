/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/x86/fpu.h>
#include <mem/vmm.h>
#include <platform/generic/system.h>
#include <platform/x86/gdt.h>
#include <platform/x86/tasking/switchvm.h>
#include <platform/x86/tasking/tss.h>

/* switching the page dir and tss to the current proc */
void switchuvm(thread_t* thread)
{
    system_disable_interrupts();
    gdt[GDT_SEG_TSS] = GDT_SEG_BG(SEGTSS_TYPE, &tss, sizeof(tss) - 1, 0);
    uintptr_t esp0 = ((uintptr_t)thread->tf + sizeof(trapframe_t));
    tss.esp0 = esp0;
    tss.ss0 = (GDT_SEG_KDATA << 3);
    tss.iomap_offset = sizeof(tss);
    RUNNING_THREAD = thread;
    fpu_make_unavail();
    set_ltr(GDT_SEG_TSS << 3);
    vmm_switch_address_space(thread->process->address_space);
    system_enable_interrupts();
}