#!/usr/bin/env python3
#
# Copyright 2021 Nikita Melekhin. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Launch the script from root of the project to have the correct paths

import subprocess
import os
import sys

connections = [
    ["libs/libapi/includes/libapi/window_server/Connections/ws_connection.ipc",
        "libs/libapi/includes/libapi/window_server/Connections/WSConnection.h"],
]

for conn in connections:
    inf = conn[0]
    outf = conn[1]
    print("Compiling {0} -> {1}", inf, outf)
    cmd = ["utils/compilers/ConnectionCompiler/connc"]
    cmd.extend([inf, outf])
    result = subprocess.run(cmd, stdout=subprocess.PIPE)
    print(result.stdout.decode("ascii"))
    print()
