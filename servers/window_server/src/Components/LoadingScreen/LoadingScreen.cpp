/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "LoadingScreen.h"
#include <libg/Context.h>
#include <libg/ImageLoaders/PNGLoader.h>

namespace WinServer {

LoadingScreen* s_WinServer_LoadingScreen_the = nullptr;

LoadingScreen::LoadingScreen()
    : m_screen(Screen::the())
{
    s_WinServer_LoadingScreen_the = this;

    LG::PNG::PNGLoader loader;
    m_logo = loader.load_from_file("/res/system/logo_dark_128.png");

    int content_min_x = m_screen.bounds().mid_x() - (m_logo.bounds().width() / 2);
    int content_min_y = m_screen.bounds().mid_y() - ((m_logo.bounds().height() + progress_line_height()) / 2);

    m_progress_line_min_x = m_screen.bounds().mid_x() - (progress_line_width() / 2);
    m_progress_line_min_y = content_min_y + m_logo.bounds().height();

    LG::Context ctx(m_screen.display_bitmap());
    ctx.draw({ content_min_x, content_min_y }, m_logo);
}

void LoadingScreen::display_status_bar(int progress, int out_of)
{
    LG::Context ctx(m_screen.display_bitmap());
    int widthp = (progress * progress_line_width()) / out_of;

    ctx.set_fill_color(LG::Color(20, 20, 20));
    ctx.fill_rounded(LG::Rect(m_progress_line_min_x, m_progress_line_min_y, progress_line_width(), progress_line_height()), LG::CornerMask(4));

    ctx.set_fill_color(LG::Color::White);
    ctx.fill_rounded(LG::Rect(m_progress_line_min_x, m_progress_line_min_y, widthp, progress_line_height()), LG::CornerMask(4));
}

} // namespace WinServer
