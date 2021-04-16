# Copyright 2021 Nikita Melekhin. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import glob
import sys
import json
import subprocess
from datetime import datetime

def shell(cmd):
    return subprocess.check_output(cmd, shell=True).decode("ascii")

outpath = sys.argv[1]
arch = sys.argv[2]
host = sys.argv[3]
branch = "{0}@{1}".format(shell("git rev-parse --short HEAD")[:-1], shell("git rev-parse --abbrev-ref HEAD")[:-1])


config = {}
config['arch'] = arch
config['host'] = host
config['branch'] = branch
config['time'] = datetime.today().strftime('%Y-%m-%d %H:%M:%S')


# Printing to the file
def print_header(config_file):
    config_file.write(
        """#
# Automatically generated file; DO NOT EDIT.
# oneOS Kernel Configuration
#

""")


def print_json(config_file, rdict):
    json.dump(rdict, config_file, indent = 4)


config_file = open(outpath, "w")
print_header(config_file)
print_json(config_file, config)
config_file.close()
