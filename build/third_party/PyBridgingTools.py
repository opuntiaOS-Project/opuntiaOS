# Copyright (C) 2020-2022 The opuntiaOS Project Authors.
#  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
from sre_constants import ASSERT
import subprocess
import sys
from pathlib import Path
from git import Repo

class PyBridgingTools:
    def __init__(self):
        pass

    @staticmethod
    def build_descriptor():
        desc = {
            "outpath": os.path.abspath(sys.argv[1]),
            "rootdir": os.path.abspath(sys.argv[2]),
            "target_cpu": sys.argv[3],
            "host": sys.argv[4],
            "toolchain": {
                "ar": sys.argv[5].split(" ")[0],
                "cc": sys.argv[5].split(" ")[1],
                "cxx": sys.argv[5].split(" ")[2],
                "ld": sys.argv[5].split(" ")[3],
                "asm": sys.argv[5].split(" ")[4],
                "target": sys.argv[5].split(" ")[5],
            },
            "c_flags": sys.argv[6] + " " if sys.argv[6] != "__EMPTY__" else "",
            "cc_flags": sys.argv[7] + " " if sys.argv[7] != "__EMPTY__" else "",
            "ld_flags": sys.argv[8] + " " if sys.argv[8] != "__EMPTY__" else "",
        }
        return desc