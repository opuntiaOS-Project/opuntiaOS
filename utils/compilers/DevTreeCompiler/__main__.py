#!/usr/bin/env python3
#
# Copyright (C) 2020-2022 The opuntiaOS Project Authors.
#  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
# 
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from Parser.Parser import Parser
from Generator.IRManager import IRManager
from Generator.BinWriter import BinWriter
import argparse

def run(input_f, output_f):
    parser = Parser(input_f)
    irmng = IRManager(parser)
    binw = BinWriter(irmng, output_f)
    binw.process()
    
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('i', type=str, help='Input file')
    parser.add_argument('o', type=str, help='Output file')

    args = parser.parse_args()
    run(args.i, args.o)
