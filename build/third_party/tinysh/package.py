# Copyright (C) 2020-2022 The opuntiaOS Project Authors.
#  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# fmt: off
import sys
from os import path
sys.path.append(path.dirname(path.dirname(path.abspath(__file__))))
from PortingTools import *
from PyBridgingTools import *
# fmt: on

state = PyBridgingTools.build_descriptor()


class TinyshPackage:
    version = "1.0"
    name = "tinysh"
    exec_name = "simple-c-shell"
    rootdir = state["rootdir"]
    target_dir = state["rootdir"] + "/third_party/tinysh"
    url = "https://github.com/jmreyes/simple-c-shell.git"

    def __init__(self):
        pass

    def clean(self):
        src_dir = self.target_dir + "/src"
        PortTools.run_command("rm -rf {0}".format(src_dir))

    def has_build(self):
        cache_location = self.target_dir + \
            "/bin_{0}/".format(state['target_arch'])
        file_in_cache = cache_location + TinyshPackage.exec_name
        return os.path.exists(file_in_cache)

    def download(self):
        PortTools.clone_git(self.target_dir, self.url)

    def apply_patches(self):
        txt, err = PortTools.apply_patch(
            self.target_dir, "0001-llvm_support.patch")
        if err:
            print(txt)
            exit(1)
        txt, err = PortTools.apply_patch(
            self.target_dir, "0002-disable_unsupported.patch")
        if err:
            print(txt)
            exit(1)

    def build(self):
        cache_location = self.target_dir + \
            "/bin_{0}/".format(state['target_arch'])
        file_in_cache = cache_location + TinyshPackage.exec_name
        if self.has_build():
            return

        src_dir = self.target_dir + "/src"

        builder = StaticBuiler(state["rootdir"], src_dir, libs=["libc"])
        PortTools.set_env("CC", state["toolchain"]["cc"])
        PortTools.set_env("LD", state["toolchain"]["ld"])

        cflags = ""
        ldflags = ""
        if state["host"] == "llvm":
            cflags += "-target " + state["toolchain"]["target"] + " "

        cflags += state["c_flags"]
        cflags += builder.libs_include_flags()
        PortTools.set_env("CFLAGS", cflags)

        ldflags += state["ld_flags"]
        ldflags += builder.libs_link_flags()
        PortTools.set_env("LDFLAGS", ldflags)

        _, err = builder.run_command(
            "make CC=\"$CC\" LD=\"$LD\" CFLAGS=\"$CFLAGS\" LDFLAGS=\"$LDFLAGS\"")
        if err:
            exit(1)

        exepath = src_dir + "/" + TinyshPackage.exec_name
        builder.move_exec(exepath, cache_location)
        builder.run_command("make clean")

    def bin_is_ready(self):
        cache_location = self.target_dir + \
            "/bin_{0}/".format(state['target_arch'])
        file_in_cache = cache_location + TinyshPackage.exec_name
        if os.path.exists(state["outpath"]):
            return
        os.symlink(file_in_cache, state["outpath"])


package = TinyshPackage()
if not package.has_build():
    package.download()
    package.apply_patches()
    package.build()
package.bin_is_ready()
