/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <libipc/Decodable.h>
#include <libipc/Encodable.h>
#include <libipc/Encoder.h>
#include <sys/types.h>

namespace LG {

class Size : public Encodable<Size>, public Decodable<Size> {
public:
    Size() = default;

    Size(size_t width, size_t height)
        : m_width(width)
        , m_height(height)
    {
    }
    ~Size() = default;

    inline void set(const Size& p) { m_width = p.width(), m_height = p.height(); }
    inline void set_width(int w) { m_width = w; }
    inline void set_height(int h) { m_height = h; }

    inline size_t width() const { return m_width; }
    inline size_t height() const { return m_height; }

    void encode(EncodedMessage& buf) const override
    {
        Encoder::append(buf, m_width);
        Encoder::append(buf, m_height);
    }

    void decode(const char* buf, size_t& offset) override
    {
        Encoder::decode(buf, offset, m_width);
        Encoder::decode(buf, offset, m_height);
    }

private:
    size_t m_width { 0 };
    size_t m_height { 0 };
};

} // namespace LG
