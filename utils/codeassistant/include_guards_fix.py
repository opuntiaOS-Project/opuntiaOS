#!/usr/bin/env python
#
# Copyright 2021 Nikita Melekhin. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Launch the script from root of the project to have the correct paths

import os
import sys
from os import fdopen, remove

walk_dir = sys.argv[1]

print('walk_dir = ' + walk_dir)
print('walk_dir (absolute) = ' + os.path.abspath(walk_dir))

all_includes = []


def is_guard(line):
    if line.startswith("#ifndef _"):
        return True
    return False


def get_guard(line):
    return line[8:-1]


def new_guard(line, path):
    gen = path.split('/')
    gen = list(filter(lambda a: a != "libs", gen))
    gen = list(filter(lambda a: a != "include", gen))
    line = "_"
    for l in gen:
        line += l + "_"
    line = line.replace(".", "_")
    line = line.replace("-", "_")
    line = line.upper()
    return line[:-1]


def fix_guards(file):
    print("prc ", file)
    data = []
    guard = None
    with open(file) as old_file:
        for line in old_file:
            data.append(line)
            if is_guard(line) and guard is None:
                guard = get_guard(line)

    if guard is None:
        return

    ng = new_guard(guard, file)

    with open(file, 'w') as new_file:
        for i in data:
            i = i.replace(guard, ng)
            new_file.write(i)


for root, subdirs, files in os.walk(walk_dir):
    for x in files:
        if x.endswith(".h") or x.endswith(".hpp") or root.find("/libcxx/include") != -1:
            fix_guards(os.path.join(root, x))
