/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <libipc/Decodable.h>
#include <libipc/Encodable.h>
#include <libipc/Encoder.h>
#include <string>
#include <sys/types.h>

namespace LIPC {

class StringEncoder : public Encodable<StringEncoder>, public Decodable<StringEncoder> {
public:
    StringEncoder() = default;
    StringEncoder(const std::string& str)
        : m_str(str)
    {
    }

    StringEncoder(std::string&& str)
        : m_str(std::move(str))
    {
    }

    void encode(EncodedMessage& buf) const override
    {
        for (int i = 0; i < m_str.size(); i++) {
            buf.push_back(m_str.at(i));
        }
        buf.push_back('\0');
    }

    void decode(const char* buf, size_t& offset) override
    {
        while (buf[offset] != '\0') {
            m_str.push_back(buf[offset]);
            offset++;
        }
        offset++;
    }
    std::string& string() { return m_str; }
    const std::string& string() const { return m_str; }
    std::string&& move_string() { return std::move(m_str); }

private:
    std::string m_str {};
};
} // namespace LIPC