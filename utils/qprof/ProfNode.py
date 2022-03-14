# Copyright (C) 2020-2022 The opuntiaOS Project Authors.
#  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from AddrResolver import AddrResolver


class Profiler:
    def __init__(self, filename):
        self.addr_resolver = AddrResolver(filename)

    def resolver(self):
        return self.addr_resolver


class ProfNode:
    def __init__(self, profiler, pc=0):
        self.profiler = profiler
        self.pc = pc
        self.name = "NOT SET"
        self.call_count = 0
        self.children = {}
        self.children_by_name = {}

    def merge(self, node):
        self.call_count += node.call_count
        self.children = {**self.children, **node.children}

    def add_stacktrace(self, stacktrace):
        if len(stacktrace) == 0:
            return
        child_pc = stacktrace[-1]
        if child_pc in self.children.keys():
            self.children[child_pc].call_count += 1
        else:
            self.children[child_pc] = ProfNode(self.profiler, child_pc)
            self.children[child_pc].call_count += 1

        self.children[child_pc].add_stacktrace(stacktrace[:-1])

    def process_node(self):
        named_nodes = {}

        for k, v in self.children.items():
            v.process_node()

            func_name = self.profiler.resolver().get(k)
            if func_name in named_nodes.keys():
                named_nodes[func_name].merge(v)
            else:
                named_nodes[func_name] = ProfNode(self.profiler)
                named_nodes[func_name].name = func_name
                named_nodes[func_name].merge(v)
        self.children = named_nodes

    def print(self, tabs=0):
        print('{} {:>4} {}'.format("    "*tabs, self.call_count, self.name))

    def trace(self, tabs=0):
        self.print(tabs)

        rd = sorted(self.children.values(),
                    key=lambda node: node.call_count, reverse=True)
        for v in rd:
            v.trace(tabs + 1)
