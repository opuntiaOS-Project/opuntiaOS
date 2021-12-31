/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Popup.h"
#include "../../Constants/Colors.h"
#include "../../Managers/WindowManager.h"
#include "../Helpers/TextDrawer.h"
#include <algorithm>

namespace WinServer {

Popup* s_WinServer_Popup_the = nullptr;

Popup::Popup()
{
    s_WinServer_Popup_the = this;
}

void Popup::on_set_data()
{
    size_t max_width = 0;
    size_t max_height = 0;
    for (auto& item : m_data) {
        max_width = std::max(max_width, Helpers::text_width(item.text, m_font));
        max_height += m_font.glyph_height() + spacing();
    }
    max_width = std::max(max_width, min_width());
    bounds().set_width(max_width + 2 * spacing());
    bounds().set_height(max_height + spacing());
}

void Popup::draw(LG::Context& ctx)
{
    if (!visible()) {
        return;
    }

    ctx.set_fill_color(LG::Color::LightSystemWhiteOpaque);
    ctx.fill_rounded(bounds(), LG::CornerMask(LG::CornerMask::SystemRadius));

    ctx.set_fill_color(Color::Shadow);
    ctx.draw_box_shading(bounds(), LG::Shading(LG::Shading::Type::Box, 0, LG::Shading::SystemSpread), LG::CornerMask(LG::CornerMask::SystemRadius));

    const size_t line_height = (m_font.glyph_height() + spacing());
    int height = bounds().min_y() + spacing();

    for (int i = 0; i < m_data.size(); i++) {
        if (i == m_hovered_item) {
            ctx.set_fill_color(LG::Color::White);
            ctx.fill_rounded(LG::Rect(bounds().min_x() + 4, height - spacing() / 2, bounds().width() - 8, line_height), LG::CornerMask(2));
        }
        ctx.set_fill_color(LG::Color::DarkSystemText);
        Helpers::draw_text(ctx, { bounds().min_x() + spacing(), height }, m_data[i].text, m_font);
        height += line_height;
    }
}

void Popup::set_preferred_origin(const LG::Point<int>& origin)
{
    auto& wm = WindowManager::the();
    LG::Point<int> pos;
    int x = origin.x();
    int y = origin.y();

    if (origin.y() + bounds().height() > wm.visible_area().max_y()) {
        y = origin.y() - bounds().height();
    }
    if (origin.x() + bounds().width() > wm.visible_area().max_x()) {
        x = origin.x() - bounds().width();
    }
    pos.set_x(x);
    pos.set_y(y);
    m_bounds.set_origin(std::move(pos));
}

} // namespace WinServer