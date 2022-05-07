# Copyright 2021 Nikita Melekhin. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import glob
import sys
# sys.argv[2] Target to generate for

platforms = ['x86', 'aarch32', 'aarch64']

ignore_platforms = []

for platform in platforms:
    if sys.argv[2] != platform:
        ignore_platforms.append(platform)


def is_file_type(name, ending):
    if len(name) <= len(ending):
        return False
    return (name[-len(ending)-1::] == '.'+ending)


def is_file_blocked(name):
    global ignore_platforms
    for platform in ignore_platforms:
        if (name.find(platform) != -1):
            return True
    return False


if sys.argv[2] == "aarch64":
    print("//kernel/kernel/platform/aarch64/interrupts/interrupts.S")
    print("//kernel/kernel/drivers/aarch64/uart.c")
    print("//kernel/kernel/drivers/generic/screen.c")
    print("//kernel/kernel/libkern/log.c")
    print("//kernel/kernel/libkern/printf.c")
    print("//kernel/kernel/libkern/scanf.c")
    print("//kernel/kernel/libkern/kassert.c")
    print("//kernel/kernel/libkern/utils.c")
    print("//kernel/kernel/libkern/mem.c")
    print("//kernel/kernel/platform/aarch64/system.c")
    print("//kernel/kernel/kernel.c")
    print("//kernel/kernel/algo/bitmap.c")
    print("//kernel/kernel/mem/pmm.c")
    print("//kernel/kernel/platform/aarch64/init.c")
    print("//kernel/kernel/platform/aarch64/interrupts/handlers.c")
    print("//kernel/kernel/drivers/aarch64/timer.c")
    print("//kernel/kernel/drivers/aarch64/gicv2.c")
    print("//kernel/kernel/drivers/arm/pl111.c")
    print("//kernel/kernel/drivers/arm/pl181.c")

    print("//kernel/kernel/platform/aarch64/prekernel/qemuboot.c")
    print("//kernel/kernel/platform/aarch64/prekernel/prekernel.S")
    print("//kernel/kernel/platform/aarch64/prekernel/prekernel_vm.c")
    print("//kernel/kernel/mem/bits64/vmm64.c")
    print("//kernel/kernel/mem/bits64/vm_pspace64.c")
    print("//kernel/kernel/mem/vm_alloc.c")
    print("//kernel/kernel/mem/kmemzone.c")
    print("//kernel/kernel/platform/aarch64/vmm/mmu.c")
    print("//kernel/kernel/tasking/cpu.c")

    print("//kernel/kernel/platform/aarch64/system_helper.S")
    print("//kernel/kernel/mem/kmalloc.c")
    print("//kernel/kernel/drivers/devtree.c")
    print("//kernel/kernel/drivers/driver_manager.c")
    print("//kernel/kernel/algo/dynamic_array.c")

    print("//kernel/kernel/tasking/tasking.c")
    print("//kernel/kernel/tasking/proc.c")
    print("//kernel/kernel/tasking/thread.c")
    print("//kernel/kernel/tasking/kthread.c")
    print("//kernel/kernel/tasking/sched.c")
    print("//kernel/kernel/tasking/elf.c")

    print("//kernel/kernel/fs/dentry.c")
    print("//kernel/kernel/fs/file.c")
    print("//kernel/kernel/fs/helper.c")
    print("//kernel/kernel/fs/vfs.c")
    print("//kernel/kernel/fs/devfs/devfs.c")
    print("//kernel/kernel/fs/ext2/ext2.c")
    print("//kernel/kernel/fs/procfs/pid.c")
    print("//kernel/kernel/fs/procfs/procfs.c")
    print("//kernel/kernel/fs/procfs/root.c")
    print("//kernel/kernel/fs/procfs/sysctl.c")

    print("//kernel/kernel/mem/memzone.c")
    print("//kernel/kernel/mem/vm_address_space.c")
    print("//kernel/kernel/libkern/umem.c")
    print("//kernel/kernel/algo/ringbuffer.c")

    print("//kernel/kernel/platform/aarch64/tasking/switch_contexts.S")
    print("//kernel/kernel/platform/aarch64/tasking/tasking_jumper.S")
    print("//kernel/kernel/platform/aarch64/tasking/switchvm.c")

    print("//kernel/kernel/time/time_manager.c")
    print("//kernel/kernel/io/shared_buffer/shared_buffer.c")
    print("//kernel/kernel/io/sockets/socket.c")
    print("//kernel/kernel/io/sockets/local_socket.c")
    print("//kernel/kernel/io/tty/ptmx.c")
    print("//kernel/kernel/io/tty/pty_master.c")
    print("//kernel/kernel/io/tty/pty_slave.c")
    print("//kernel/kernel/io/tty/tty.c")
    print("//kernel/kernel/io/tty/vconsole.c")

    print("//kernel/kernel/drivers/generic/ramdisk.c")

    print("//kernel/kernel/syscalls/fs.c")
    print("//kernel/kernel/syscalls/handler.c")
    print("//kernel/kernel/syscalls/identity.c")
    print("//kernel/kernel/syscalls/io.c")
    print("//kernel/kernel/syscalls/ptrace.c")
    print("//kernel/kernel/syscalls/system.c")
    print("//kernel/kernel/syscalls/tasking.c")
    print("//kernel/kernel/syscalls/time.c")
    print("//kernel/kernel/tasking/blocker.c")

    print("//kernel/kernel/drivers/aarch64/fpu.c")
    print("//kernel/kernel/drivers/aarch64/fpu_helper.S")

    print("//kernel/kernel/drivers/arm/aplfb.c")
else:
    for path, subdirs, files in os.walk("../kernel/kernel"):
        for name in files:
            # It runs from out dir, at least it should
            file = "//" + path[3:] + "/" + name
            if not is_file_type(file, 'c') and not is_file_type(file, 's') and not is_file_type(file, 'S'):
                continue
            if is_file_blocked(file):
                continue
            print(file)


def special_paths(platform):
    pass


special_paths(sys.argv[2])
