/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */


#ifndef __oneOS__X86__TASKING__SCHED_H
#define __oneOS__X86__TASKING__SCHED_H

#include <mem/vmm/vmm.h>
#include <tasking/tasking.h>
#include <types.h>
#include <x86/idt.h>

#define MAX_PRIO 0
#define MIN_PRIO 10
#define PRIOS_COUNT (MIN_PRIO - MAX_PRIO + 1)
#define DEFAULT_PRIO 6
#define SCHED_INT 100

void scheduler_init();
void resched_dont_save_context();
void resched();
void sched();
void sched_enqueue(thread_t* thread);
void sched_dequeue(thread_t* thread);

#endif // __oneOS__X86__TASKING__SCHED_H