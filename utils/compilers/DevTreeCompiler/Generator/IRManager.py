# Copyright (C) 2020-2022 The opuntiaOS Project Authors.
#  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

class IRManager():
    def __init__(self, parser):
        self.parser = parser

    def platform_name(self):
        return self.parser.data()["name"]

    def device_count(self):
        return len(self.parser.data()["devices"])

    def device_list(self):
        return self.parser.data()["devices"]
