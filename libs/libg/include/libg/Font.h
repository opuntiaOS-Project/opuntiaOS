/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <libg/Color.h>
#include <libg/PixelBitmap.h>
#include <libg/Rect.h>
#include <sys/types.h>

namespace LG {

class Font;
class GlyphBitmap {
public:
    friend class Font;
    GlyphBitmap() = default;
    GlyphBitmap(const uint32_t* rows, uint8_t width, uint8_t height)
        : m_rows(rows)
        , m_width(width)
        , m_height(height)
    {
    }

    ~GlyphBitmap() = default;

    inline bool bit_at(int x, int y) const { return row(y) & (1 << x); }
    inline size_t width() const { return m_width; }
    inline size_t height() const { return m_height; }

    inline const uint32_t* rows() const { return m_rows; }
    inline uint32_t row(uint32_t index) const { return m_rows[index]; }
    inline bool empty() { return !m_rows || !m_width || !m_height; }

private:
    const uint32_t* m_rows { nullptr };
    uint8_t m_width { 0 };
    uint8_t m_height { 0 };
};

class Font {
public:
    enum Type {
        Default = 0,
        LatinExtendedA = 1
    };

    Font(uint32_t* raw_data, uint8_t* width_data, uint8_t width, uint8_t height, size_t count, bool dynamic_width, uint8_t glyph_spacing);
    ~Font() = default;

    static Font& system_font();
    static Font& system_bold_font();
    static Font* load_from_file(const char* path);
    static Font* load_from_mem(uint8_t* path);

    inline size_t glyph_width(size_t ch) const { return m_dynamic_width ? m_width_data[ch] : m_width; }
    inline size_t glyph_height() const { return m_height; }
    inline size_t glyph_spacing() const { return m_spacing; }
    GlyphBitmap glyph_bitmap(size_t ch) const;

private:
    uint32_t* m_raw_data;
    uint8_t* m_width_data;
    size_t m_width;
    size_t m_height;
    size_t m_spacing;
    size_t m_count;
    bool m_dynamic_width;
};

} // namespace LG