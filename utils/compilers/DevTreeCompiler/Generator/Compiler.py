# Copyright (C) 2020-2022 The opuntiaOS Project Authors.
#  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from Generator.IRManager import IRManager
from ABI.Structs import *
from ABI.Translation import *


class Compiler():
    __debug = False

    def __init__(self, irmng):
        self.irmng = irmng

        self.res_binarr = bytearray()
        self.header_binarr = bytearray()
        self.devs_binarr = bytearray()
        self.names_binarr = bytearray()

    def build_header(self):
        count_of_devs = len(self.irmng.device_list())

        result = {
            "signature": DEVTREE_HEADER_SIGNATURE,
            "revision": 1,
            "flags": 0,
            "entries_count": count_of_devs,
            "name_list_offset": DEVTREE_HEADER.sizeof() + len(self.devs_binarr),
        }

        self.header_binarr = DEVTREE_HEADER.build(result)

    def build_dev(self, dev):
        result = {
            "type": 0,
            "flags": 0,
            "region_base": 0,
            "region_size": 0,
            "irq_lane": 0,
            "irq_flags": 0,
            "irq_priority": 0,
            "rel_name_offset": len(self.names_binarr),
        }

        if "type" in dev:
            result["type"] = Translator.entry_type(dev["type"])

        if "flags" in dev:
            result["flags"] = Translator.flags(
                dev["flags"], Translator.entry_flag_translator)

        if "mem" in dev:
            devmem = dev["mem"]
            if "base" in devmem:
                result["region_base"] = Translator.number(devmem["base"])
            if "size" in devmem:
                result["region_size"] = Translator.number(devmem["size"])

        if "irq" in dev:
            devint = dev["irq"]
            if "lane" in devint:
                result["irq_lane"] = Translator.number(devint["lane"])
            if "flags" in devint:
                result["irq_flags"] = Translator.flags(
                    devint["flags"], Translator.irq_flag_translator)
            if "priority" in devint:
                result["irq_priority"] = Translator.number(devint["priority"])

        self.devs_binarr += DEVTREE_ENTRY.build(result)
        self.names_binarr += bytearray((map(ord,
                                       dev["name"]))) + bytearray([0])

    def build_dev_list(self):
        self.devs_binarr = bytearray()
        self.names_binarr = bytearray()
        for dev in self.irmng.device_list():
            self.build_dev(dev)

    def build_binarr(self):
        self.res_binarr = bytearray()
        self.header_binarr = bytearray()
        self.devs_binarr = bytearray()
        self.names_binarr = bytearray()

        self.build_dev_list()
        self.build_header()

        self.res_binarr = self.header_binarr + self.devs_binarr + self.names_binarr

        if self.__debug:
            print("Header", self.header_binarr)
            print("Devs", self.devs_binarr)
            print("Names", self.names_binarr)
            print("Res", self.res_binarr)

    def compile(self):
        if len(self.res_binarr) == 0:
            self.build_binarr()

        return self.res_binarr
