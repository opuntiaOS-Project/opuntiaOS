# Copyright (C) 2020-2022 The opuntiaOS Project Authors.
#  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
# 
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from Parser.Parser import Parser
from Generator.IRManager import IRManager
from Generator.BinWriter import BinWriter
from Generator.CWriter import CWriter
from Generator.Compiler import Compiler
import argparse

class DevTreeCompiler():
    
    @staticmethod
    def compile(input_f, output_f):
        parser = Parser(input_f)
        irmng = IRManager(parser)
        compiler = Compiler(irmng)
        binw = BinWriter(output_f)
        binw.write(compiler.compile())

    @staticmethod
    def to_c_arr(input_f):
        parser = Parser(input_f)
        irmng = IRManager(parser)
        compiler = Compiler(irmng)
        binw = CWriter()
        binw.write(compiler.compile())