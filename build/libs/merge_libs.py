# Copyright 2021 Nikita Melekhin. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import random
import subprocess
import sys
target_lib = sys.argv[1]
srcs_lib = list(sys.argv[2:])

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

    output = subprocess.check_output("i686-elf-ar -M <{0}".format(filename), shell=True)
    output = subprocess.check_output("rm {0}".format(filename), shell=True)