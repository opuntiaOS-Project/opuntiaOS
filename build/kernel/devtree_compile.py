# Copyright 2021 Nikita Melekhin. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import glob
import sys
import json
import subprocess
from datetime import datetime

OBJCOPY_TOOL = ""
OBJCOPY_TARGET = ""


def shell(cmd, cwd=None):
    return subprocess.check_output(cmd, shell=True, cwd=cwd).decode("ascii")


inpath = sys.argv[1]
outpath = sys.argv[2]
arch = sys.argv[3]
board = sys.argv[4]
host = sys.argv[5]
path_to_bins = sys.argv[6]

if path_to_bins == "__EMPTY_PATH_":
    path_to_bins = ""
if len(path_to_bins) != 0:
    if path_to_bins[-1] != '/':
        path_to_bins += "/"

if (arch == "aarch32"):
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
elif (arch == "aarch64"):
    if host == "gnu":
        OBJCOPY_TOOL = "{0}aarch64-elf-objcopy".format(path_to_bins)
        OBJCOPY_TARGET = "elf64-littleaarch64"
    elif host == "llvm":
        OBJCOPY_TOOL = "{0}llvm-objcopy".format(path_to_bins)
        OBJCOPY_TARGET = "elf64-littleaarch64"
else:
    print("Unsupported arch {0}".format(arch))
    exit(1)

run_from = os.getcwd() + '/../utils/compilers/DevTreeCompiler'
inpath_abs = os.getcwd() + '/' + inpath
outpath_abs = os.getcwd() + '/' + outpath
obj_outpath_abs = outpath_abs + "o"

shell("python3 . {0} {1}".format(inpath_abs, outpath_abs), run_from)
shell("{0} -I binary -O {1} --rename-section .data=.odt {2} {3}".format(
    OBJCOPY_TOOL, OBJCOPY_TARGET, outpath_abs, obj_outpath_abs))
