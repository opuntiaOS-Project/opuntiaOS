/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/generic/init.h>
#include <platform/generic/registers.h>
#include <platform/generic/system.h>

#include <libkern/types.h>

#include <drivers/driver_manager.h>

#include <mem/kmalloc.h>
#include <mem/pmm.h>

#include <fs/devfs/devfs.h>
#include <fs/ext2/ext2.h>
#include <fs/procfs/procfs.h>
#include <fs/vfs.h>

#include <io/shared_buffer/shared_buffer.h>
#include <io/tty/ptmx.h>
#include <io/tty/tty.h>

#include <time/time_manager.h>

#include <tasking/sched.h>

#include <libkern/log.h>

#include <syscalls/handlers.h>

static int __boot_cpu_launched = 0;
static int __boot_cpu_setup_devices = 0;
static int __boot_cpu_setup_drivers = 0;
static int __boot_cpu_setup_tasking = 0;

static inline void boot_cpu_finish(int* wt)
{
    __atomic_store_n(wt, 1, __ATOMIC_RELEASE);
}

static inline void wait_for_boot_cpu_to_finish(int* wt)
{
    while (__atomic_load_n(wt, __ATOMIC_ACQUIRE) == 0) {
    }
}

void launching()
{
    tasking_create_kernel_thread(dentry_flusher, NULL);
    tasking_start_init_proc();
    ksys1(SYS_EXIT, 0);
}

void stage3(mem_desc_t* mem_desc)
{
    boot_cpu_finish(&__boot_cpu_launched);
    system_disable_interrupts();
    logger_setup();
    platform_init_boot_cpu();

    // mem setup
    pmm_setup(mem_desc);
    vmm_setup();
    platform_setup_boot_cpu();
    boot_cpu_finish(&__boot_cpu_setup_devices);

    // installing drivers
    driver_manager_init();
    platform_drivers_setup();
    timeman_setup();
    vfs_install();
    ext2_install();
    procfs_install();
    devfs_install();
    drivers_run();
    boot_cpu_finish(&__boot_cpu_setup_drivers);

    // mounting filesystems
    procfs_mount();
    devfs_mount();

    // ipc
    shared_buffer_init();

    // pty
    ptmx_install();

    // init scheduling
    tasking_init();
    scheduler_init();
    schedule_activate_cpu();
    tasking_create_kernel_thread(launching, NULL);
    boot_cpu_finish(&__boot_cpu_setup_tasking);
    resched(); /* Starting a scheduler */

    system_stop();
}

void boot_secondary_cpu()
{
    system_disable_interrupts();

    wait_for_boot_cpu_to_finish(&__boot_cpu_setup_devices);
    vmm_setup_secondary_cpu();
    platform_setup_secondary_cpu();

    wait_for_boot_cpu_to_finish(&__boot_cpu_setup_tasking);
    schedule_activate_cpu();
    resched();

    system_stop();
}