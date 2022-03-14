# Copyright (C) 2020-2022 The opuntiaOS Project Authors.
#  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from QMP import QEMUMonitorProtocol
import re


class QConn:
    def __init__(self, path):
        self.qconn = QEMUMonitorProtocol(path)
        self.qconn.connect()
        self.runs = True
        self.regs_cache = ""

    def drop_cache(self):
        self.regs_cache = ""

    def stop(self):
        self.human_cmd("stop")
        self.runs = False
        self.drop_cache()

    def cont(self):
        self.human_cmd("cont")
        self.runs = True

    def gpreg(self, name):
        if self.runs or self.regs_cache == "":
            self.regs_cache, err = self.human_cmd("info registers")
        name = name.upper()
        fd = re.search("{}=([\w]+)".format(name), self.regs_cache)
        if fd is None:
            return ""
        return fd.group(1)

    def human_cmd(self, line):
        args = {}
        args["command-line"] = line
        resp = self.qconn.cmd("human-monitor-command", args)
        if "error" in resp:
            return (resp["error"]["desc"], 1)
        return (resp["return"], 0)
