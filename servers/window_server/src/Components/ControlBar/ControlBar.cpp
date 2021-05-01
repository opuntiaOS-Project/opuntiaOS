/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ControlBar.h"
#include "../../Compositor.h"
#include <libg/ImageLoaders/PNGLoader.h>

namespace WinServer {

static ControlBar* s_the;

ControlBar& ControlBar::the()
{
    return *s_the;
}

ControlBar::ControlBar()
    : m_bounds(0, Screen::the().bounds().height() - height(), Screen::the().bounds().width(), height())
{
    s_the = this;
    LG::PNG::PNGLoader loader;
    m_logo = loader.load_from_file("/res/system/mobile/control.png");
    int x = width() / 2 - m_logo.width() / 2;
    int y = Screen::the().bounds().height() - height() + height() / 2 - m_logo.height() / 2;
    m_button_bounds = LG::Rect(x, y, m_logo.width(), m_logo.height());
}

} // namespace WinServer