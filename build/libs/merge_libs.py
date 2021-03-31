# Copyright 2021 Nikita Melekhin. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

AR_TOOL = ""

import random
import subprocess
import sys
arch = sys.argv[1]
host = sys.argv[2]
target_lib = sys.argv[3]
srcs_lib = list(sys.argv[4:])

if (arch == "aarch32"):
    AR_TOOL = "arm-none-eabi-ar"
    if host == "gnu":
        srcs_lib.append("../toolchains/gcc_runtime/10.2.1/arm-none-eabi-libgcc.a")
    elif host == "llvm":
        srcs_lib.append("../toolchains/llvm_runtime/11.1.0/libclang_rt.builtins-arm.a")
elif (arch == "x86"):
    AR_TOOL = "i686-elf-ar"
    srcs_lib.append("../toolchains/llvm_runtime/11.1.0/libclang_rt.builtins-i386.a")

if (len(srcs_lib) == 1):
    output = subprocess.check_output("cp {1} {0}".format(target_lib, srcs_lib[0]), shell=True)
else:
    filename = "libmerger{0}.mri".format(random.randint(10000, 100000))

    ffile = open(filename, "w")
    ffile.write("CREATE {0}\n".format(target_lib))
    for i in srcs_lib:
        ffile.write("ADDLIB {0}\n".format(i))
    ffile.write("SAVE\n")
    ffile.write("END")
    ffile.close()

    output = subprocess.check_output("{0} -M <{1}".format(AR_TOOL, filename), shell=True)
    output = subprocess.check_output("rm {0}".format(filename), shell=True)