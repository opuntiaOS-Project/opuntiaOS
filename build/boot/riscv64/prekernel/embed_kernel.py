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
    OBJCOPY_TOOL = "{0}riscv64-unknown-elf-objcopy".format(path_to_bins)
    OBJCOPY_TARGET = "elf64-littleriscv"
elif host == "llvm":
    OBJCOPY_TOOL = "{0}llvm-objcopy".format(path_to_bins)
    OBJCOPY_TARGET = "elf64-littleriscv"
else:
    print("Unsupported host {0}".format(host))
    exit(0)

cmd = "{0} -I binary -O {1} --rename-section .data=.kernelelf {2} {3}".format(
    OBJCOPY_TOOL, OBJCOPY_TARGET, prekernel_path, out_path)
output = subprocess.check_output(cmd, shell=True)
