# Copyright 2022 Nikita Melekhin. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys
import os
import subprocess
from construct import *
from elftools.elf.elffile import ELFFile
from elftools.elf.constants import SH_FLAGS

prekernel_path = sys.argv[1]
out_path = sys.argv[2]
build_dir_path = os.path.dirname(os.path.abspath(sys.argv[2]))

# rawImage for x86 is a multiboot-capable elf file. Since qemu does not
# implement support for multiboot2 specs, which could load elf64, we have
# to change the type of the elf file to elf32-i686.
output = subprocess.check_output(
    "x86_64-elf-objcopy -I binary -O elf64-x86-64 --rename-section .data=.kernelelf {0} {1}".format(prekernel_path, out_path), shell=True)
