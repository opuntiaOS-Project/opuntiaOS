/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
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

    void set_shading(const LG::Shading& sh) { m_shading = sh; }
    const LG::Shading& shading() const { return m_shading; }

    void display(const LG::Rect& rect, const LG::Rect& frame) const
    {
        LG::Context ctx = graphics_current_context();
        ctx.set_fill_color(LG::Color(100, 100, 100, 40));
        ctx.add_clip(rect);
        ctx.draw_box_shading(frame, m_shading, m_corner_mask);
    }

private:
    LG::CornerMask m_corner_mask {};
    LG::Shading m_shading {};
};

} // namespace UI