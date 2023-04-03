#!/usr/bin/env python3
#
# Copyright (C) 2020-2022 The opuntiaOS Project Authors.
#  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import glob
import argparse
import shutil

parser = argparse.ArgumentParser()
parser.add_argument('target', type=str, help='Target third_part package')
parser.add_argument('--clean', action='store_true',
                    help='Cleans /src and build cache')
args = parser.parse_args()

portname = args.target
if not os.path.exists(portname):
    print("Can't find port {0}".format(portname))

if args.clean:
    if os.path.exists("{0}/src/".format(portname)):
        shutil.rmtree("{0}/src/".format(portname))
    for filename in glob.glob("{0}/patches/.applied_*".format(portname)):
        os.remove(filename)
    for arch in ["x86", "arm32", "arm64", "x86_64", "riscv64"]:
        path = "{0}/bin_{1}/".format(portname, arch)
        if os.path.exists(path):
            shutil.rmtree(path)
    exit(0)
