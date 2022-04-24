/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include <libipc/Encoder.h>

template <typename T>
class Encodable {
public:
    virtual void encode(EncodedMessage& buf) const { }
};