/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
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
    CornerMask(size_t radius, bool top_rounded = true, bool bottom_rounded = true)
        : m_radius(radius)
        , m_top_rounded(top_rounded)
        , m_bottom_rounded(bottom_rounded)
    {
    }

    size_t radius() const { return m_radius; }
    bool top_rounded() const { return m_top_rounded; }
    bool bottom_rounded() const { return m_bottom_rounded; }

private:
    size_t m_radius { 4 };
    bool m_top_rounded { true };
    bool m_bottom_rounded { true };
};

} // namespace LG