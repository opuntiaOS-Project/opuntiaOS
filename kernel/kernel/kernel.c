/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
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

/* If we stay our anythread alone it can't get keyboard input,
   so we will switch to idle_thread to fix it. */
void idle_thread()
{
    while (1) {
        ksys0(SYSSCHEDYIELD);
    }
}

void launching()
{
    tasking_create_kernel_thread(idle_thread, NULL);
    tasking_create_kernel_thread(dentry_flusher, NULL);
    tasking_start_init_proc();
    ksys1(SYSEXIT, 0);
}

void stage3(mem_desc_t* mem_desc)
{
    system_disable_interrupts();
    logger_setup();
    platform_setup();

    // mem setup
    pmm_setup(mem_desc);
    vmm_setup();

    // installing drivers
    driver_manager_init();
    platform_drivers_setup();
    timeman_setup();
    vfs_install();
    ext2_install();
    procfs_install();
    devfs_install();
    drivers_run();

    // mounting filesystems
    devfs_mount();

    // ipc
    shared_buffer_init();

    // pty
    ptmx_install();

    // init scheduling
    tasking_init();
    scheduler_init();
    tasking_create_kernel_thread(launching, NULL);
    resched(); /* Starting a scheduler */

    while (1) { }
}