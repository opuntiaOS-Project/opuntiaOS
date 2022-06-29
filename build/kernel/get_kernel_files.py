# Copyright 2021 Nikita Melekhin. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import glob
import sys
# sys.argv[2] Target to generate for
target = sys.argv[2]

platforms = ['x86', 'i386', 'x86_64', 'arm', 'arm32', 'arm64', 'aarch32', 'aarch64']
bits = ['bits32', 'bits64']

platform_to_bits = {
    "x86": "bits32",
    "x86_64": "bits64",
    "arm32": "bits32",
    "arm64": "bits64",
}

allowed_paths = {
    "x86": ["x86", "i386"],
    "x86_64": ["x86", "x86_64"],
    "arm32": ["aarch32", "arm32", "arm"],
    "arm64": ["aarch64", "arm64", "arm"],
}

if target == "x86_64":
    print("//kernel/kernel/platform/x86_64/entry.s")
    exit(0)

ignore_platforms = []
ignore_bits = []

allowed_paths_for_target = allowed_paths.get(target, None)
if allowed_paths_for_target is None:
    print("Unknown paltform {0}".format(target))
    exit(1)

for platform in platforms:
    if not (platform in allowed_paths_for_target):
        ignore_platforms.append(platform)

for bit in bits:
    if platform_to_bits[target] != bit:
        ignore_bits.append(bit)


def is_file_type(name, ending):
    if len(name) <= len(ending):
        return False
    return (name[-len(ending)-1::] == '.'+ending)


def is_file_blocked(name):
    global ignore_platforms
    for platform in ignore_platforms:
        if (name.find(platform) != -1):
            return True
    for bit in ignore_bits:
        if (name.find(bit) != -1):
            return True
    return False


for path, subdirs, files in os.walk("../kernel/kernel"):
    for name in files:
        # It runs from out dir, at least it should
        file = "//" + path[3:] + "/" + name
        if not is_file_type(file, 'c') and not is_file_type(file, 's') and not is_file_type(file, 'S'):
            continue
        if is_file_blocked(file):
            continue
        print(file)


def special_paths(platform):
    pass


special_paths(target)
