# Copyright (C) 2020-2022 The opuntiaOS Project Authors.
#  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
# 
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import json

class Parser():
    def __init__(self, input_file):
        self.decoded_data = None
        self.input_file = input_file

    def data(self):
        if self.decoded_data is None:
            f = open(self.input_file)
            self.decoded_data = json.load(f)        
            f.close()
            
        return self.decoded_data


    



