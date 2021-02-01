# Copyright 2021 Nikita Melekhin. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys
# sys.argv[2] Target to generate for

platforms = ['x86', 'aarch32']

ignore_platforms = []

for platform in platforms:
    if sys.argv[2] != platform:
        ignore_platforms.append(platform)

def is_file_type(name, ending):
    if len(name) <= len(ending):
        return False
    return (name[-len(ending)-1::] == '.'+ending)

def is_file_blocked(name):
    global ignore_platforms
    for platform in ignore_platforms:
        if (name.find(platform) != -1):
            return True
    return False

import glob, os
for path, subdirs, files in os.walk("../src/kernel"):
    for name in files:
        # It runs from out dir, at least it should
        file = "//" + path[3:] + "/" + name
        if not is_file_type(file, 'c') and not is_file_type(file, 's'):
            continue
        if is_file_blocked(file):
            continue
        print(file)

def special_paths(paltform):
    if paltform == 'x86':
        print("//src/boot/x86/stage3_entry.s")
    if paltform == 'aarch32':
        print("//src/boot/aarch32/vm_init.c")
        print("//src/boot/aarch32/_start.s")

special_paths(sys.argv[2])