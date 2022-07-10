# Copyright 2022 Nikita Melekhin. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys
import os
import subprocess

prekernel_path = sys.argv[1]
out_path = sys.argv[2]
arch = sys.argv[3]
board = sys.argv[4]
host = sys.argv[5]
path_to_bins = sys.argv[6]

if path_to_bins == "__EMPTY_PATH_":
    path_to_bins = ""
if len(path_to_bins) != 0:
    if path_to_bins[-1] != '/':
        path_to_bins += "/"

if host == "gnu":
    OBJCOPY_TOOL = "{0}x86_64-elf-objcopy".format(path_to_bins)
elif host == "llvm":
    OBJCOPY_TOOL = "{0}llvm-objcopy".format(path_to_bins)
else:
    print("Unsupported host {0}".format(host))
    exit(0)

# rawImage for x86 is a multiboot-capable elf file. Since qemu does not
# implement support for multiboot2 specs, which could load elf64, we have
# to change the type of the elf file to elf32-i686.
output = subprocess.check_output(
    "{0} -O elf32-i386 {1} {2}".format(OBJCOPY_TOOL, prekernel_path, out_path), shell=True)
