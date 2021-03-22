/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
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
    static ResourceManager& the();
    ResourceManager();

    inline const LG::PixelBitmap& background() const { return m_background; }

private:
    LG::PixelBitmap m_background;
};

} // namespace WinServer