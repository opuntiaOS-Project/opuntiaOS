# Copyright 2021 Nikita Melekhin. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys
import os
import json
import subprocess

outpath = sys.argv[1]
app_name = sys.argv[2]


def print_json(config_file, rdict):
    json.dump(rdict, config_file, indent=4)


def write_config(outpath):
    config_file = open(outpath+"/info.json", "w")
    config = {}

    config['name'] = app_name

    print_json(config_file, config)
    config_file.close()


if not os.path.exists(outpath):
    os.makedirs(outpath)
write_config(outpath)
