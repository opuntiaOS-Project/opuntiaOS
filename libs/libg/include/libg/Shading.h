/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <sys/types.h>

namespace LG {

class Shading {
public:
    enum Type {
        LeftToRight,
        RightToLeft,
        TopToBottom,
        BottomToTop,
        Deg45,
        Deg135,
        Deg225,
        Deg315,
        Box,
    };
    static const int SystemSpread = 4;

    Shading() = delete;
    Shading(Type type, uint8_t final_alpha)
        : m_type(type)
        , m_final_alpha(final_alpha)
    {
    }

    Shading(Type type, uint8_t final_alpha, int spread)
        : m_type(type)
        , m_final_alpha(final_alpha)
        , m_spread(spread)
    {
    }

    ~Shading() = default;

    inline Type type() const { return m_type; }
    inline uint8_t final_alpha() const { return m_final_alpha; }
    inline int spread() const { return m_spread; }

private:
    Type m_type { LeftToRight };
    uint8_t m_final_alpha { 0 };
    int m_spread { SystemSpread };
};

} // namespace LG