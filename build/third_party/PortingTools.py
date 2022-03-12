# Copyright (C) 2020-2022 The opuntiaOS Project Authors.
#  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import subprocess
import sys
from pathlib import Path
from git import Repo


class PortTools:
    def __init__(self):
        pass

    @staticmethod
    def set_env(name, val):
        os.environ[name] = str(val)

    @staticmethod
    def run_command_impl(cmd, cwd="."):
        result = subprocess.run(
            cmd, stdout=subprocess.PIPE, shell=True, cwd=cwd)
        return (result.stdout.decode("ascii"), result.returncode)

    @staticmethod
    def run_command(cmd, cwd="."):
        return PortTools.run_command_impl(cmd, cwd)

    @staticmethod
    def apply_patch(location, patch_name):
        src_dir = location+"/src"
        if not os.path.exists(src_dir):
            return ("", -1)

        patch_path = location+"/patches/"+patch_name
        if not os.path.exists(patch_path):
            return ("", -1)

        applied_patch_path = location+"/patches/.applied_"+patch_name
        if os.path.exists(applied_patch_path):
            return ("", 0)

        cmd = ["patch"]
        myinput = open(patch_path)
        result = subprocess.run(
            cmd, stdin=myinput, stdout=subprocess.PIPE, cwd=src_dir)
        if result.returncode == 0:
            Path(applied_patch_path).touch()
        return (result.stdout.decode("ascii"), result.returncode)

    @staticmethod
    def clone_git(location, url):
        src_dir = location+"/src"
        if os.path.exists(src_dir):
            return
        Repo.clone_from(url, src_dir)


class StaticBuiler:
    def check_libs_present(self, lib_name):
        lib_desc = StaticBuiler.libs.get(lib_name, None)
        if lib_desc is None:
            return False

        if not os.path.exists(lib_desc["bin"]):
            return False

        return True

    def __init__(self, rootdir, srcdir, libs):
        self.rootdir = rootdir
        self.srcdir = srcdir
        self.target_libs = libs
        self.libs = {
            "libc": {
                "include": self.rootdir + "/libs/libc/include",
                "bin": self.rootdir + "/out/base/libs/",
            }
        }

    def libs_include_flags(self):
        flags = ""
        for lib_name in self.target_libs:
            lib_desc = self.libs.get(lib_name, None)
            assert(lib_desc is not None)

            flags += "-I"
            flags += lib_desc["include"] + " "
        return flags

    def libs_link_flags(self):
        flags = " "
        for lib_name in self.target_libs:
            lib_desc = self.libs.get(lib_name, None)
            assert(lib_desc is not None)

            flags += "-L"
            flags += lib_desc["bin"] + " "
            flags += "-l" + lib_name[3:] + " "
        return flags

    def run_command(self, cmd_str):
        return PortTools.run_command(cmd_str, self.srcdir)

    def move_exec(self, execname, targetdir):
        if not os.path.exists(targetdir):
            os.makedirs(targetdir)
        return self.run_command("mv {0} {1}".format(execname, targetdir))


if __name__ == '__main__':
    pass
