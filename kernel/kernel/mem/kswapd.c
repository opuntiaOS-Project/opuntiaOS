/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <fs/vfs.h>
#include <libkern/bits/errno.h>
#include <libkern/log.h>
#include <mem/kswapd.h>
#include <mem/swapfile.h>
#include <mem/vmm.h>
#include <syscalls/handlers.h>
#include <tasking/tasking.h>

// #define KSWAPD_DEBUG
#define KSWAPD_SLEEPTIME (10) // seconds.

static kmemzone_t _mapzone;
static uintptr_t _mmaped_ptable;
extern proc_t proc[MAX_PROCESS_COUNT];

static int map_ptable(table_desc_t* ptable_desc)
{
    uintptr_t ptable_paddr = PAGE_START((uintptr_t)table_desc_get_frame(*ptable_desc));
    if (_mmaped_ptable == ptable_paddr) {
        return 0;
    }

    int err = vmm_map_page(_mapzone.start, ptable_paddr, PAGE_READABLE | PAGE_WRITABLE);
    if (err) {
        return err;
    }

    _mmaped_ptable = ptable_paddr;
    return 0;
}

static int find_victim(proc_t* p, pdirectory_t* pdir)
{
    lock_acquire(&p->vm_lock);
    ptable_t* ptable_map_zone = (ptable_t*)_mapzone.ptr;
    const size_t ptables_per_page = VMM_PAGE_SIZE / PTABLE_SIZE;
    const size_t table_coverage = VMM_PAGE_SIZE * VMM_TOTAL_PAGES_PER_TABLE;

    for (int pti = 0; pti < VMM_KERNEL_TABLES_START; pti++) {
        table_desc_t* ptable_desc = &pdir->entities[pti];
        if (!table_desc_has_attrs(*ptable_desc, TABLE_DESC_PRESENT)) {
            continue;
        }

        map_ptable(ptable_desc);
        for (int pgi = 0; pgi < VMM_TOTAL_PAGES_PER_TABLE; pgi++) {
            page_desc_t* ppage_desc = &ptable_map_zone[pti].entities[pgi];
            if (!page_desc_has_attrs(*ppage_desc, PAGE_DESC_PRESENT)) {
                continue;
            }

            uintptr_t victim_vaddr = table_coverage * pti + VMM_PAGE_SIZE * pgi;
#ifdef KSWAPD_DEBUG
            log("[kswapd] (pid %d) Find victim at %x", p->pid, victim_vaddr);
#endif
            vmm_swap_page(&ptable_map_zone[pti], NULL, victim_vaddr);
            lock_release(&p->vm_lock);
            return 0;
        }
    }

    lock_release(&p->vm_lock);
    return 0;
}

void kswapd()
{
    _mapzone = kmemzone_new(VMM_PAGE_SIZE);

    for (;;) {
        if (pmm_get_free_space_in_kb() * 4 >= pmm_get_ram_in_kb()) {
#ifdef KSWAPD_DEBUG
            log("[kswapd] Skip pass, %d free of %d, <75%% busy", pmm_get_free_space_in_kb(), pmm_get_ram_in_kb());
#endif
            goto sleep;
        }

        proc_t* p;
        for (int i = 0; i < tasking_get_proc_count(); i++) {
            p = &proc[i];
            if (p->status == PROC_ALIVE) {
                find_victim(p, p->pdir);
            }
        }

    sleep:
        ksys1(SYS_NANOSLEEP, KSWAPD_SLEEPTIME);
    }
}