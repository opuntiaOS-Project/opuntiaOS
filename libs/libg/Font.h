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
#include "Rect.h"
#include <libcxx/sys/types.h>

namespace LG {

enum FontTypes {
    Default = 0,
    LatinExtendedA = 1
};

class Font;
class GlyphBitmap {
public:
    friend class Font;
    ~GlyphBitmap() { }

    inline bool bit_at(int x, int y) const { return row(y) & (1 << x); }
    inline size_t width() const { return m_width; }
    inline size_t height() const { return m_height; }

    inline const uint32_t* rows() const { return m_rows; }
    inline uint32_t row(uint32_t index) const { return m_rows[index]; }

private:
    GlyphBitmap(const uint32_t* rows, uint8_t width, uint8_t height)
        : m_rows(rows)
        , m_width(width)
        , m_height(height)
    {
    }

    const uint32_t* m_rows { nullptr };
    uint8_t m_width { 0 };
    uint8_t m_height { 0 };
};

class Font {
public:
    Font(uint32_t* raw_data, uint8_t* m_width_data, uint8_t width, uint8_t height, size_t count, bool dynamic_width);
    ~Font() { }

    static Font& system_font();
    static Font* load_from_file(const char* path);
    static Font* load_from_mem(uint8_t* path);

    uint8_t glyph_width(size_t ch) const { return m_dynamic_width ? m_width_data[ch] : m_width; }
    GlyphBitmap glyph_bitmap(size_t ch) const;

private:
    uint32_t* m_raw_data;
    uint8_t* m_width_data;
    uint8_t m_width;
    uint8_t m_height;
    size_t m_count;
    bool m_dynamic_width;
};

}