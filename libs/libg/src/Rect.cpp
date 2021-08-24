/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libg/Rect.h>

namespace LG {

Rect::Rect(int x, int y, size_t width, size_t height)
    : m_origin(x, y)
    , m_width(width)
    , m_height(height)
{
}

void Rect::encode(EncodedMessage& buf) const
{
    Encoder::append(buf, m_origin);
    Encoder::append(buf, m_width);
    Encoder::append(buf, m_height);
}

void Rect::decode(const char* buf, size_t& offset)
{
    Encoder::decode(buf, offset, m_origin);
    Encoder::decode(buf, offset, m_width);
    Encoder::decode(buf, offset, m_height);
}

} // namespace LG