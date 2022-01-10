# Copyright (C) 2020-2022 The opuntiaOS Project Authors.
#  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
# 
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from construct import *

DEVTREE_HEADER = Struct(
    "signature" / PaddedString(8, "ascii"),
    "flags" / Int32ul,
    "entries_count" / Int32ul,
    "name_list_offset" / Int32ul
)


DEVTREE_ENTRY_FLAGS_MMIO = 0x1

DEVTREE_ENTRY = Struct(
    "type" / Int32ul,
    "flags" / Int32ul,
    "paddr" / Int32ul,
    "rel_name_offset" / Int32ul,
)
