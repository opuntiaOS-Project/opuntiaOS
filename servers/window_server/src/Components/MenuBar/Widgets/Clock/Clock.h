/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "../../../Helpers/TextDrawer.h"
#include "../BaseWidget.h"
#include "../../MenuBar.h"
#include <libfoundation/Logger.h>
#include <libg/Font.h>

namespace WinServer {

class Clock : public BaseWidget {
public:
    Clock()
    {
        m_date = (char*)malloc(10);
        memset(m_date, 0, 10);
        memcpy(m_date, "Mon 09:41", 9);
    }

    ~Clock()
    {
        free(m_date);
    }

    size_t width() override { return 80; }
    void draw(LG::Context& ctx) override
    {
        ctx.set_fill_color(LG::Color::Black);
        size_t twidth = Helpers::text_width(m_date, m_font);
        Helpers::draw_text(ctx, { int(width() - twidth) / 2, 6 }, m_date, m_font);
    }

    WidgetAnswer click_began(int x, int y) override
    {
        m_clicked = true;
        return WidgetAnswer::InvalidateMe;
    }

    WidgetAnswer click_ended() override
    {
        m_clicked = false;
        return WidgetAnswer::InvalidateMe;
    }

private:
    bool m_clicked { false };
    char* m_date { nullptr };
    LG::Font& m_font { LG::Font::system_font() };
};

} // namespace WinServer