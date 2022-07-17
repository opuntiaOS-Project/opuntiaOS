# Copyright (C) 2020-2022 The opuntiaOS Project Authors.
#  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# fmt: off
import zipfile
import sys
import os
from os import path
sys.path.append(path.dirname(path.dirname(path.dirname(path.abspath(__file__)))) + "/build/third_party/")
from PortingTools import *
from PyBridgingTools import *
# fmt: on

VERSION = "2.10.0"

state = PyBridgingTools.build_descriptor()

lib_path = state["rootdir"] + "/libs/libfreetype"
if not os.path.exists(lib_path + "/include/libfreetype"):
    with zipfile.ZipFile(lib_path + "/bin/libfreetype_headers.zip", "r") as zip_ref:
        zip_ref.extractall(lib_path + "/include/")


if not os.path.exists(state["outpath"]):
    file_in_cache = state["rootdir"] + \
        "/libs/libfreetype/bin/libfreetype_{0}.a".format(state['target_arch'])
    os.symlink(file_in_cache, state["outpath"])
