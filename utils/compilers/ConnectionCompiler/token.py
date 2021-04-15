# Copyright 2021 Nikita Melekhin. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

class Token:

    def __init__(self, type=None, value=None):
        self.type = type
        self.value = value

    def __str__(self):
        return "Token({0}, {1})".format(self.type, self.value)