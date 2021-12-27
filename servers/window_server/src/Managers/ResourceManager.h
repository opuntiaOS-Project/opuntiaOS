/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include <libg/PixelBitmap.h>
#include <libg/Point.h>

namespace WinServer {

class ResourceManager {
public:
    inline static ResourceManager& the()
    {
        extern ResourceManager* s_WinServer_ResourceManager_the;
        return *s_WinServer_ResourceManager_the;
    }

    ResourceManager();

    inline const LG::PixelBitmap& background() const { return m_background; }

private:
    LG::PixelBitmap m_background;
};

} // namespace WinServer