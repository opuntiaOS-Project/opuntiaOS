/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#pragma once

#include "Color.h"
#include "PixelBitmap.h"
#include "Point.h"
#include "Rect.h"
#include <libcxx/malloc.h>
#include <libcxx/sys/types.h>

namespace LG {

class Context {
public:
    Context(PixelBitmap&);
    ~Context() { }

    void add_clip(const Rect& rect);
    void reset_clip();

    void draw(const Point<int>& start, const PixelBitmap& bitmap);
    void fill(const Rect& rect);

private:
    PixelBitmap& m_bitmap;
    Rect m_clip;
    const Rect m_origin_clip;
};

}