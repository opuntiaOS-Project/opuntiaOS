/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <libfreetype/freetype/freetype.h>
#include <libg/Color.h>
#include <libg/Glyph.h>
#include <libg/PixelBitmap.h>
#include <libg/Rect.h>
#include <sys/types.h>

namespace LG {

class FontCacher {
public:
    FontCacher() = default;
    ~FontCacher() = default;

    void cache(int ch, Glyph&& gl) { m_cache[ch] = std::move(gl); }
    const Glyph& get(int ch) const { return m_cache[ch]; }
    bool has(int ch) const { return !m_cache[ch].empty(); }

private:
    Glyph m_cache[256];
};

class Font {
public:
    static const int SystemDefaultSize = 10;
    static const int SystemTitleSize = 20;
    static const int SystemMaxSize = 36;

    ~Font() = default;

    static Font& system_font(int of_size = SystemDefaultSize);
    static Font& system_bold_font(int of_size = SystemDefaultSize);
    static Font* load_from_file(const char* path);
    static Font* load_from_file_ttf(const char* path, size_t size);
    static Font* load_from_mem(uint8_t* path);

    inline size_t size() const { return m_font_size; }
    inline const Glyph& glyph(size_t ch) const
    {
        if (!m_font_cache->has(ch)) {
            m_font_cache->cache(ch, load_glyph(ch));
        }
        return m_font_cache->get(ch);
    }

private:
    enum FontType {
        SerenityOS,
        FreeType,
    };

    struct SerenityOSFontDesc final {
        uint32_t* raw_data;
        uint8_t* width_data;
        uint8_t height;
        uint8_t width;
        uint8_t spacing;
        bool dynamic_width;
        size_t count;

        Glyph load_glyph(size_t ch) const;
    };

    struct FreeTypeFontDesc final {
        FT_Face face;
        size_t height;

        Glyph load_glyph(size_t ch) const;
    };

    Font(const SerenityOSFontDesc& font_desc, size_t font_size);
    Font(const FreeTypeFontDesc& font_desc, size_t font_size);

    Glyph load_glyph(size_t ch) const;

    size_t m_font_size { 0 };
    size_t m_const_width { 0 }; // If const width is 0, width id calculated dynamicly for each glyph.

    FontCacher* m_font_cache { nullptr };

    FontType m_font_type;
    union {
        SerenityOSFontDesc serenity;
        FreeTypeFontDesc free_type;
    } m_font_desc;
};

} // namespace LG