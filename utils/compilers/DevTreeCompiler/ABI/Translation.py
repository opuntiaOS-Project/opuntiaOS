# Copyright (C) 2020-2022 The opuntiaOS Project Authors.
#  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from ABI.Structs import *


class Translator():

    @staticmethod
    def entry_flag(s):
        translation = {
            "MMIO": DEVTREE_ENTRY_FLAGS_MMIO,
        }
        return translation.get(s, 0)

    @staticmethod
    def entry_type(s):
        translation = {
            "IO": DEVTREE_ENTRY_TYPE_IO,
            "FB": DEVTREE_ENTRY_TYPE_FB,
            "UART": DEVTREE_ENTRY_TYPE_UART,
            "RAM": DEVTREE_ENTRY_TYPE_RAM,
            "STORAGE": DEVTREE_ENTRY_TYPE_STORAGE,
            "BUS_CONTROLLER": DEVTREE_ENTRY_TYPE_BUS_CONTROLLER
        }
        return translation.get(s, DEVTREE_ENTRY_TYPE_IO)

    @staticmethod
    def number(s):
        return int(s, base=0)

    @staticmethod
    def entry_flags(s):
        flags = 0x0
        ents = s.split(" ")

        for ent in ents:
            t = Translator.entry_flag(s)
            if t != None:
                flags |= t

        return flags
