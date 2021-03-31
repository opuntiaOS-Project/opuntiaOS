#!/usr/bin/env python
#
# Copyright 2021 Nikita Melekhin. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import subprocess
import os
import sys


class ClassTidyLauncher():
    def __init__(self, dir, includes):
        self.path_dir = dir
        self.include = includes
        self.front_flags = ["--use-color", "--fix"]
        self.back_flags = ["-c", "-m32",
                           "-D_LIBCXX_BUILD_ONEOS_EXTENSIONS"]  # 32bit flag

    def run_clang_tidy(self, ff, files, bf):
        cmd = ["clang-tidy"]
        cmd.extend(ff)
        cmd.extend(files)
        cmd.extend(["--"])
        cmd.extend(bf)
        result = subprocess.run(cmd, stdout=subprocess.PIPE)
        return result.stdout

    def process_includes(self):
        for i in self.include:
            self.back_flags.append("-I")
            self.back_flags.append(i)

    def get_files(self):
        self.c_files = []
        self.cpp_files = []
        platforms = ["x86", "aarch32"]
        ignore_platforms = []

        for platform in platforms:
            if "x86" != platform:
                ignore_platforms.append(platform)

        def is_file_type(name, ending):
            if len(name) <= len(ending):
                return False
            return (name[-len(ending)-1::] == '.'+ending)

        def is_file_blocked(name):
            for platform in ignore_platforms:
                if (name.find(platform) != -1):
                    return True
            return False

        for path, subdirs, files in os.walk(self.path_dir):
            for name in files:
                # It runs from out dir, at least it should
                file = path + "/" + name
                if is_file_blocked(file):
                    continue
                if is_file_type(file, 'c'):
                    self.c_files.append(file)
                if is_file_type(file, 'cpp'):
                    self.cpp_files.append(file)

    def process(self):
        self.process_includes()
        self.get_files()
        self.c_back_flags = self.back_flags
        self.c_back_flags += ["-std=gnu99"]
        ret = ""
        if len(self.c_files) > 0:
            ret += self.run_clang_tidy(self.front_flags,
                                       self.c_files, self.c_back_flags).decode("ascii")
        self.cpp_back_flags = self.back_flags
        self.cpp_back_flags += ["-std=c++2a"]
        if len(self.cpp_files) > 0:
            ret += self.run_clang_tidy(self.front_flags,
                                       self.cpp_files, self.cpp_back_flags).decode("ascii")
        return ret


kernel_includes = ["kernel/include"]
app_includes = ["libs/libc/include", "libs/libcxx/include", "libs/libfoundation/include",
                "libs/libipc/include", "libs/libg/include", "libs/libui/include"]

print(ClassTidyLauncher("kernel/kernel", kernel_includes).process())
print(ClassTidyLauncher("servsers/", app_includes).process())
print(ClassTidyLauncher("libs/", app_includes).process())
