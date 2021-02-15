/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#pragma once

#include "Color.h"
#include "Font.h"
#include "PixelBitmap.h"
#include "Point.h"
#include "Rect.h"
#include "Shading.h"
#include <libcxx/malloc.h>
#include <libcxx/sys/types.h>

namespace LG {

class Context {
public:
    Context(PixelBitmap&);
    ~Context() { }

    void add_clip(const Rect& rect);
    void reset_clip();

    void set(const Point<int>& start, const PixelBitmap& bitmap);
    void draw(const Point<int>& start, const PixelBitmap& bitmap);
    void draw(const Point<int>& start, const GlyphBitmap& bitmap);
    void draw_shading(const Rect& rect, const Shading& shading);
    void fill(const Rect& rect);
    void mix(const Rect& rect);
    void add_ellipse(const Rect& rect);

    void set_draw_offset(const Point<int>& offset) { m_draw_offset = offset; }
    void set_fill_color(const Color& clr) { m_color = clr; }

    inline const Color& fill_color() const { return m_color; }

private:
    PixelBitmap& m_bitmap;
    Rect m_clip;
    const Rect m_origin_clip;
    Point<int> m_draw_offset { 0, 0 };
    Color m_color {};
};

}