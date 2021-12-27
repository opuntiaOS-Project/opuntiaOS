/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ResourceManager.h"
#include <libg/ImageLoaders/PNGLoader.h>

namespace WinServer {

ResourceManager* s_WinServer_ResourceManager_the = nullptr;

ResourceManager::ResourceManager()
{
    s_WinServer_ResourceManager_the = this;
    LG::PNG::PNGLoader loader;
    m_background = loader.load_from_file("/res/wallpapers/new_year.png");
}

} // namespace WinServer