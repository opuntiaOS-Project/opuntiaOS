/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ResourceManager.h"
#include <libg/ImageLoaders/PNGLoader.h>

namespace WinServer {

static ResourceManager* s_the;

ResourceManager& ResourceManager::the()
{
    return *s_the;
}

ResourceManager::ResourceManager()
{
    s_the = this;
    LG::PNG::PNGLoader loader;
    m_background = loader.load_from_file("/res/wallpapers/island.png");
}

} // namespace WinServer