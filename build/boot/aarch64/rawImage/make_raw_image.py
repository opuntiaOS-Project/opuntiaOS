# Copyright 2022 Nikita Melekhin. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys
inf = sys.argv[1]
outf = sys.argv[2]
reli = sys.argv[3]

import subprocess
output = subprocess.check_output("cat {0} {1} > {2}".format(inf, outf, reli), shell=True)