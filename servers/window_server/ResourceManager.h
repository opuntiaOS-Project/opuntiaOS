/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#pragma once
#include <libg/PixelBitmap.h>
#include <libg/Point.h>

class ResourceManager {
public:
    static ResourceManager& the();
    ResourceManager();

    inline const LG::PixelBitmap& background() const { return m_background; }
private:
    LG::PixelBitmap m_background;
};