/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "CursorManager.h"
#include <libg/ImageLoaders/PNGLoader.h>

namespace WinServer {

CursorManager* s_WinServer_CursorManager_the = nullptr;

CursorManager::CursorManager()
    : m_screen(Screen::the())
{
    s_WinServer_CursorManager_the = this;
    LG::PNG::PNGLoader loader;
#ifdef TARGET_DESKTOP
    m_std_cursor = loader.load_from_file("/res/system/arrow.png");
#elif TARGET_MOBILE
    m_std_cursor = loader.load_from_file("/res/system/mobile/cursor.png");
#endif
}

} // namespace WinServer