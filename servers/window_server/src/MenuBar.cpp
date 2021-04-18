/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "MenuBar.h"
#include <libg/ImageLoaders/PNGLoader.h>

namespace WinServer {

static MenuBar* s_the;

MenuBar& MenuBar::the()
{
    return *s_the;
}

MenuBar::MenuBar()
    : m_background_color(LG::Color(222, 222, 222, 180))
{
    s_the = this;
    LG::PNG::PNGLoader loader;
    m_logo = loader.load_from_file("/res/system/logo_dark_12.png");
}

} // namespace WinServer