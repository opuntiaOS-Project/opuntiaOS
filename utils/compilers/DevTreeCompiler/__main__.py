#!/usr/bin/env python3
#
# Copyright (C) 2020-2022 The opuntiaOS Project Authors.
#  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from DevTreeCompiler import DevTreeCompiler
import argparse

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('i', type=str, help='Input file')
    parser.add_argument('o', type=str, help='Output file')
    parser.add_argument('--dumpc', action='store_true',
                        help='Dumping C array')

    args = parser.parse_args()

    if args.dumpc:
        DevTreeCompiler.to_c_arr(args.i)
    else:
        DevTreeCompiler.compile(args.i, args.o)
