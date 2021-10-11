/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Popup.h"
#include "../Helpers/TextDrawer.h"
#include <algorithm>

namespace WinServer {

Popup* s_WinServer_Popup_the = nullptr;

Popup::Popup()
{
    s_WinServer_Popup_the = this;
}

void Popup::set_data(PopupData& data)
{
    size_t max_width = 0;
    size_t max_height = 0;
    for (auto& item : data) {
        max_width = std::max(max_width, Helpers::text_width(item.text, m_font));
        max_height += m_font.glyph_height() + spacing();
    }
    max_width = std::max(max_width, min_width());
    bounds().set_width(max_width + 2 * spacing());
    bounds().set_height(max_height + spacing());
    m_data = &data;
}

void Popup::draw(LG::Context& ctx)
{
    if (!visible()) {
        return;
    }

    ctx.set_fill_color(LG::Color::White);
    ctx.fill_rounded(bounds(), LG::CornerMask(LG::CornerMask::SystemRadius));

    const size_t line_height = (m_font.glyph_height() + spacing());
    int height = bounds().min_y() + spacing();
    auto& data = *m_data;

    for (int i = 0; i < data.size(); i++) {
        if (i == m_hovered_item) {
            ctx.set_fill_color(LG::Color::LightSystemAccentButton);
            ctx.fill(LG::Rect(bounds().min_x(), height - spacing() / 2, bounds().width(), line_height));
            ctx.set_fill_color(LG::Color::White);
        } else {
            ctx.set_fill_color(LG::Color::DarkSystemText);
        }
        Helpers::draw_text(ctx, { bounds().min_x() + spacing(), height }, data[i].text, m_font);
        height += line_height;
    }
}

} // namespace WinServer