/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <cstddef>
#include <sys/types.h>

namespace LG {

class CornerMask {
public:
    static constexpr bool Masked = true;
    static constexpr bool NonMasked = false;
    static constexpr size_t SystemRadius = 4;

    CornerMask() = default;
    CornerMask(size_t radius, bool top_rounded = Masked, bool bottom_rounded = Masked)
        : m_radius(radius)
        , m_top_rounded(top_rounded)
        , m_bottom_rounded(bottom_rounded)
    {
    }

    ~CornerMask() = default;

    static CornerMask Standard() { return CornerMask(SystemRadius, Masked, Masked); }

    size_t radius() const { return m_radius; }
    bool top_rounded() const { return m_top_rounded; }
    bool bottom_rounded() const { return m_bottom_rounded; }

private:
    size_t m_radius { 0 };
    bool m_top_rounded { Masked };
    bool m_bottom_rounded { Masked };
};

} // namespace LG