# Copyright 2021 Nikita Melekhin. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys
import os
import subprocess

OBJCOPY_TOOL = ""
OBJCOPY_TARGET = ""


def shell(cmd, cwd=None):
    return subprocess.check_output(cmd, shell=True, cwd=cwd).decode("ascii")


outpath = sys.argv[1]
arch = sys.argv[2]
board = sys.argv[3]
host = sys.argv[4]
path_to_bins = sys.argv[5]

if path_to_bins == "__EMPTY_PATH_":
    path_to_bins = ""
if len(path_to_bins) != 0:
    if path_to_bins[-1] != '/':
        path_to_bins += "/"

if (arch == "arm32"):
    if host == "gnu":
        OBJCOPY_TOOL = "{0}arm-none-eabi-objcopy".format(path_to_bins)
        OBJCOPY_TARGET = "elf32-littlearm"
    elif host == "llvm":
        OBJCOPY_TOOL = "{0}llvm-objcopy".format(path_to_bins)
        OBJCOPY_TARGET = "elf32-littlearm"
elif (arch == "x86"):
    if host == "gnu":
        OBJCOPY_TOOL = "{0}i686-elf-objcopy".format(path_to_bins)
        OBJCOPY_TARGET = "elf32-i386"
    elif host == "llvm":
        OBJCOPY_TOOL = "{0}llvm-objcopy".format(path_to_bins)
        OBJCOPY_TARGET = "elf32-i386"
elif (arch == "x86_64"):
    if host == "gnu":
        OBJCOPY_TOOL = "{0}x86_64-elf-objcopy".format(path_to_bins)
        OBJCOPY_TARGET = "elf64-x86-64"
    elif host == "llvm":
        OBJCOPY_TOOL = "{0}llvm-objcopy".format(path_to_bins)
        OBJCOPY_TARGET = "elf64-x86-64"
elif (arch == "arm64"):
    if host == "gnu":
        OBJCOPY_TOOL = "{0}aarch64-elf-objcopy".format(path_to_bins)
        OBJCOPY_TARGET = "elf64-littleaarch64"
    elif host == "llvm":
        OBJCOPY_TOOL = "{0}llvm-objcopy".format(path_to_bins)
        OBJCOPY_TARGET = "elf64-littleaarch64"
else:
    print("Unsupported arch {0}".format(arch))
    exit(0)

outpath_abs = os.getcwd() + '/' + outpath

codesign_section_len = 1024

tmp_empty_bin_name = 'elfsign_section.bin'
f = open(tmp_empty_bin_name, "wb")
f.seek(codesign_section_len-1)
f.write(b"\0")
f.close()


shell("{0} -I binary -O {1} --rename-section .data=._signature {2} {3}".format(
    OBJCOPY_TOOL, OBJCOPY_TARGET, tmp_empty_bin_name, outpath_abs))
shell("{0} --set-section-flags ._signature=noload,readonly {1} {2}".format(
    OBJCOPY_TOOL, outpath_abs, outpath_abs))
