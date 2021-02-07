/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#ifndef __oneOS__X86__SYSTEM_H
#define __oneOS__X86__SYSTEM_H

#include <platform/generic/registers.h>
#include <types.h>

/**
 * INTS
 */

void system_disable_interrupts();
void system_enable_interrupts();
void system_enable_interrupts_only_counter();

/**
 * PAGING
 */

inline static void system_set_pdir(uint32_t pdir)
{
    asm volatile("mov %%eax, %%cr3"
                 :
                 : "a"(pdir));
}

inline static void system_flush_tlb_entry(uint32_t vaddr)
{
    asm volatile("invlpg (%0)" ::"r"(vaddr)
                 : "memory");
}

inline static void system_flush_whole_tlb()
{
    system_set_pdir(read_cr3());
}

inline static void system_enable_write_protect()
{
    asm volatile("mov %cr0, %eax");
    asm volatile("or $0x10000, %eax");
    asm volatile("mov %eax, %cr0");
}

inline static void system_disable_write_protect()
{
    asm volatile("mov %cr0, %eax");
    asm volatile("and $0xFFFEFFFF, %eax");
    asm volatile("mov %eax, %cr0");
}

inline static void system_enable_paging()
{
    asm volatile("mov %cr0, %eax");
    asm volatile("or $0x80000000, %eax");
    asm volatile("mov %eax, %cr0");
}

inline static void system_disable_paging()
{
    asm volatile("mov %cr0, %eax");
    asm volatile("and $0x7FFFFFFF, %eax");
    asm volatile("mov %eax, %cr0");
}

inline static void system_stop()
{
    asm volatile("cli\n");
    asm volatile("hlt\n");
}

#endif /* __oneOS__X86__SYSTEM_H */