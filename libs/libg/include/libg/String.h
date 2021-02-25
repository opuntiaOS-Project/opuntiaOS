/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <libg/Point.h>
#include <libipc/Decodable.h>
#include <libipc/Encodable.h>
#include <libipc/Encoder.h>
#include <std/String.h>
#include <sys/types.h>
#include <syscalls.h>

namespace LG {

class String : public ::String, public Encodable<String>, public Decodable<String> {
public:
    using ::String::String;

    void encode(EncodedMessage& buf) const override
    {
        for (int i = 0; i < size(); i++) {
            buf.push_back(at(i));
        }
        buf.push_back('\0');
    }

    void decode(const char* buf, size_t& offset) override
    {
        while (buf[offset] != '\0') {
            push_back(buf[offset]);
            offset++;
        }
        offset++;
    }
};
}