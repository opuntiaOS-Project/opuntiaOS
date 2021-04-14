/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "CursorManager.h"
#include <libg/ImageLoaders/PNGLoader.h>

namespace WinServer {

static CursorManager* s_the;

CursorManager& CursorManager::the()
{
    return *s_the;
}

CursorManager::CursorManager()
    : m_screen(Screen::the())
{
    s_the = this;
    LG::PNG::PNGLoader loader;
    m_std_cursor = loader.load_from_file("/res/system/arrow.png");
}

} // namespace WinServer