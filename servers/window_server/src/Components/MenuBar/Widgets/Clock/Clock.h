/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "../../../Helpers/TextDrawer.h"
#include "../../MenuBar.h"
#include "../BaseWidget.h"
#include <ctime>
#include <libfoundation/Logger.h>
#include <libg/Font.h>

#define DATA_BUF 32

namespace WinServer {

class Clock : public BaseWidget {
public:
    Clock()
    {
        m_date = static_cast<char*>(malloc(DATA_BUF));
        memset(m_date, 0, DATA_BUF);
        update_time();
        MenuBar::the().event_loop().add(LFoundation::Timer([this] {
            this->update_time();
        },
            5000, LFoundation::Timer::Repeat));
    }

    ~Clock()
    {
        free(m_date);
    }

    void update_time()
    {
        std::time_t new_time = std::time(nullptr);
        if (new_time / 60 != cur_time / 60) {
            cur_time = new_time;
            std::strftime(m_date, DATA_BUF, "%a %R", std::localtime(&cur_time));
            MenuBar::the().invalidate_widget(this);
        }
    }

    size_t width() override { return 50; }
    void draw(LG::Context& ctx) override
    {
        size_t twidth = Helpers::text_width(m_date, m_font);
        Helpers::draw_text(ctx, { int(width() - twidth) / 2, 6 }, m_date, m_font);
    }

    MenuItemAnswer mouse_down(int x, int y) override
    {
        m_clicked = true;
        return MenuItemAnswer::InvalidateMe;
    }

    MenuItemAnswer mouse_up() override
    {
        m_clicked = false;
        return MenuItemAnswer::InvalidateMe;
    }

    void popup_rect(LG::Rect& r) override { }

private:
    bool m_clicked { false };
    char* m_date { nullptr };
    std::time_t cur_time { 0 };
    LG::Font& m_font { LG::Font::system_font() };
};

} // namespace WinServer