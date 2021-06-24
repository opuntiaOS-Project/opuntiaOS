# Copyright 2021 Nikita Melekhin. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys
import os
import json
import subprocess

app_name = sys.argv[1]
outpath = sys.argv[2]
src_dir = sys.argv[3]


def print_json(config_file, rdict):
    json.dump(rdict, config_file, indent=4)


def read_config(path):
    with open(path) as json_file:
        data = json.load(json_file)
        return data
    return {}


def write_config(config, outpath):
    config_file = open(outpath+"/info.json", "w")

    config['name'] = app_name

    print_json(config_file, config)
    config_file.close()


if not os.path.exists(outpath):
    os.makedirs(outpath)

config = {}
for fname in os.listdir(src_dir):
    if fname == "info.json":
        config = read_config(src_dir + "/info.json")
        break

write_config(config, outpath)
