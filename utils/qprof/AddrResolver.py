# Copyright (C) 2020-2022 The opuntiaOS Project Authors.
#  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import subprocess
import os


class AddrResolver:
    def __init__(self, path):
        self.path = path
        self.cache = {}

    def get(self, addr):
        if addr not in self.cache:
            s = subprocess.check_output(
                "i686-elf-addr2line --demangle -fsp -e " + self.path + " " + hex(addr), shell=True)
            s = s.decode("ascii")
            self.cache[addr] = s.split(" ")[0]
        return self.cache[addr]
