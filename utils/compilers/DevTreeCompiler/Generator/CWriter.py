# Copyright (C) 2020-2022 The opuntiaOS Project Authors.
#  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


class CWriter():
    def __init__(self):
        pass

    def write(self, data):
        print("static uint8_t _devtree_raw[] = {")
        for byte in bytearray(data):
            print(hex(byte), end = ",")
        print("0x0};")