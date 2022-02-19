/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
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
#define KSWAPD_SLEEPTIME (3) // seconds.
#define KSWAPD_SWAP_PER_PID_THRESHOLD (2)
#define KSWAPD_SWAP_PER_RUN_THRESHOLD (4)

static kmemzone_t _mapzone;
static uintptr_t _mmaped_ptable;
static int moved_out_pages_per_run = 0;
static int moved_out_pages_per_pid = 0;
static int last_pid = 0, last_pti = 0;
extern proc_t proc[MAX_PROCESS_COUNT];

static int map_ptable(table_desc_t* ptable_desc)
{
    uintptr_t ptable_paddr = PAGE_START((uintptr_t)table_desc_get_frame(*ptable_desc));
    if (_mmaped_ptable == ptable_paddr) {
        return 0;
    }

    int err = vmm_map_page(_mapzone.start, ptable_paddr, MMU_FLAG_PERM_READ | MMU_FLAG_PERM_WRITE);
    if (err) {
        return err;
    }

    _mmaped_ptable = ptable_paddr;
    return 0;
}

inline static void after_page_swap()
{
    moved_out_pages_per_run++;
    moved_out_pages_per_pid++;
}

static void do_sleep()
{
    moved_out_pages_per_run = 0;
    ksys1(SYS_NANOSLEEP, KSWAPD_SLEEPTIME);
}

static int find_victim(proc_t* p, pdirectory_t* pdir)
{
    lock_acquire(&p->vm_lock);
    ptable_t* ptable_map_zone = (ptable_t*)_mapzone.ptr;
    const size_t ptables_per_page = VMM_PAGE_SIZE / PTABLE_SIZE;
    const size_t table_coverage = VMM_PAGE_SIZE * VMM_TOTAL_PAGES_PER_TABLE;

    for (int pti = last_pti; pti < VMM_KERNEL_TABLES_START; pti++, last_pti++) {
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
            memzone_t* zone = memzone_find(p->address_space, victim_vaddr);
#ifdef KSWAPD_DEBUG
            log("[kswapd] (pid %d) Find victim at %x", p->pid, victim_vaddr);
#endif
            vmm_swap_page(&ptable_map_zone[pti], zone, victim_vaddr);
            after_page_swap();
            if (moved_out_pages_per_pid >= KSWAPD_SWAP_PER_PID_THRESHOLD) {
                lock_release(&p->vm_lock);
                return 0;
            }
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
        for (int i = last_pid; i < tasking_get_proc_count(); i++, last_pid++) {
            p = &proc[i];
            if (p->status == PROC_ALIVE) {
                find_victim(p, p->address_space->pdir);
            }
            last_pti = 0;
            moved_out_pages_per_pid = 0;
            if (moved_out_pages_per_run >= KSWAPD_SWAP_PER_RUN_THRESHOLD) {
                goto sleep;
            }
        }
        last_pid = 0;

    sleep:
        do_sleep();
    }
}