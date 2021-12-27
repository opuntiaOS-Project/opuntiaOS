/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include <libg/Font.h>
#include <libui/Constants/Text.h>
#include <libui/EdgeInsets.h>
#include <libui/View.h>
#include <string>

namespace UI {

class Layer {
public:
    Layer() = default;
    ~Layer() = default;

    void set_corner_mask(const LG::CornerMask& cm) { m_corner_mask = cm; }
    const LG::CornerMask& corner_mask() const { return m_corner_mask; }

private:
    LG::CornerMask m_corner_mask {};
};

} // namespace UI