# Copyright (C) 2020-2022 The opuntiaOS Project Authors.
#  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys
import os
import subprocess
from pathlib import Path


def shell(cmd, cwd=None):
    return subprocess.check_output(cmd, shell=True, cwd=cwd).decode("ascii")


elffile_path = sys.argv[1]
stampfile_path = sys.argv[2]

run_from = os.getcwd() + '/../utils/crypto/'
elffile_path_abs = os.getcwd() + '/' + elffile_path

shell("python3 elfsign.py {0} --overwrite".format(elffile_path_abs), run_from)
Path(stampfile_path).touch()
