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
#include <string>
#include <sys/types.h>

namespace LG {

class string : public std::string, public Encodable<string>, public Decodable<string> {
public:
    using std::string::string;

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
} // namespace LG