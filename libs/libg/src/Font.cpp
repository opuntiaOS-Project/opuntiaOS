/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <fcntl.h>
#include <libfoundation/Logger.h>
#include <libg/Font.h>
#include <new>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace LG {

/* SerenityOS font header */
struct [[gnu::packed]] FontFileHeader {
    char magic[4];
    uint8_t glyph_width;
    uint8_t glyph_height;
    uint16_t range_mask_size;
    uint8_t is_variable_width;
    uint8_t glyph_spacing;
    uint8_t baseline;
    uint8_t mean_line;
    uint8_t presentation_size;
    uint16_t weight;
    uint8_t slope;
    char name[32];
    char family[32];
};

Font& Font::system_font(int size)
{
    static Font* s_system_font_ptr[SystemMaxSize + 1];
    if (!s_system_font_ptr[size]) {
        s_system_font_ptr[size] = Font::load_from_file_ttf("/GoogleSans.ttf", size);
    }
    return *s_system_font_ptr[size];
}

Font& Font::system_bold_font(int size)
{
    static Font* s_system_font_ptr[SystemMaxSize + 1];
    if (!s_system_font_ptr[size]) {
        s_system_font_ptr[size] = Font::load_from_file_ttf("/GoogleSansBold.ttf", size);
    }
    return *s_system_font_ptr[size];
}

Font::Font(const SerenityOSFontDesc& desc, size_t font_size)
    : m_font_type(FontType::SerenityOS)
    , m_font_size(font_size)
    , m_const_width(desc.dynamic_width ? 0 : desc.width)
{
    // TODO: SerenityOS bitmap fonts does not requier a cache.
    m_font_desc.serenity = desc;
    m_font_cache = new FontCacher();
}

Font::Font(const FreeTypeFontDesc& desc, size_t font_size)
    : m_font_type(FontType::FreeType)
    , m_font_size(font_size)
    , m_const_width(0)
{
    m_font_desc.free_type = desc;
    m_font_cache = new FontCacher();
}

Font* Font::load_from_file(const char* path)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        return nullptr;
    }

    stat_t stat;
    fstat(fd, &stat);

    uint8_t* ptr = (uint8_t*)mmap(NULL, stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    auto* res = Font::load_from_mem(ptr);

    close(fd);
    return res;
}

Font* Font::load_from_mem(uint8_t* font_data)
{
    if (!font_data) {
        return nullptr;
    }

    FontFileHeader& header = *(FontFileHeader*)font_data;

    if (memcmp((uint8_t*)header.magic, (const uint8_t*)"!Fnt", 4)) {
        Logger::debug << "Font unsupported" << std::endl;
        return nullptr;
    }

    size_t count = 0;
    uint8_t* range_mask = const_cast<uint8_t*>(font_data + sizeof(FontFileHeader));
    for (size_t i = 0; i < header.range_mask_size; ++i)
        count += 256 * __builtin_popcount(range_mask[i]);

    size_t bytes_per_glyph = sizeof(uint32_t) * header.glyph_height;

    uint32_t* raw_data = (uint32_t*)(range_mask + header.range_mask_size);
    uint8_t* width_data = nullptr;
    if (header.is_variable_width) {
        width_data = (uint8_t*)((uint8_t*)(raw_data) + count * bytes_per_glyph);
    }

    SerenityOSFontDesc desc = {
        .raw_data = raw_data,
        .width_data = width_data,
        .height = header.glyph_height,
        .width = header.glyph_width,
        .spacing = header.glyph_spacing,
        .dynamic_width = (bool)header.is_variable_width,
        .count = count,
    };
    return new Font(desc, header.glyph_height);
}

Font* Font::load_from_file_ttf(const char* path, size_t size)
{
    FT_Library library;
    FT_Face face;

    int error = FT_Init_FreeType(&library);
    if (error) {
        return nullptr;
    }

    error = FT_New_Face(library, path, 0, &face);
    if (error) {
        return nullptr;
    }

    error = FT_Set_Pixel_Sizes(face, 0, size);
    if (error) {
        return nullptr;
    }

    FreeTypeFontDesc desc = {
        .face = face,
        .height = size,
    };
    return new Font(desc, size);
}

Glyph Font::SerenityOSFontDesc::load_glyph(size_t ch) const
{
    size_t w = dynamic_width ? width_data[ch] : width;
    GlyphMetrics metrics = {
        .width = (uint8_t)w,
        .height = (uint8_t)height,
        .top_bearing = (uint8_t)height,
        .left_bearing = 0,
        .baseline = 0,
        .advance = (uint8_t)(w + spacing),
        .font_size = (uint16_t)height
    };
    return Glyph(&raw_data[ch * height], metrics, Glyph::ConstDataMarker {});
}

Glyph Font::FreeTypeFontDesc::load_glyph(size_t ch) const
{
    if (ch == ' ') {
        size_t width = height / 2;
        GlyphMetrics metrics = {
            .width = (uint8_t)width,
            .height = (uint8_t)height,
            .top_bearing = 0,
            .left_bearing = 0,
            .baseline = 0,
            .advance = (uint8_t)width,
            .font_size = (uint16_t)height
        };
        return Glyph(nullptr, metrics, Glyph::ConstDataMarker {});
    }

    FT_Load_Char(face, ch, FT_LOAD_RENDER);
    FT_GlyphSlot glyph = face->glyph;
    FT_Bitmap bitmap = glyph->bitmap;

    GlyphMetrics metrics = {
        .width = (uint8_t)bitmap.width,
        .height = (uint8_t)bitmap.rows,
        .top_bearing = (uint8_t)glyph->bitmap_top,
        .left_bearing = (uint8_t)glyph->bitmap_left,
        .baseline = 2,
        .advance = (uint8_t)(glyph->advance.x >> 6),
        .font_size = (uint16_t)height
    };
    return Glyph(bitmap.buffer, metrics);
}

Glyph Font::load_glyph(size_t ch) const
{
    switch (m_font_type) {
    case FontType::SerenityOS:
        return m_font_desc.serenity.load_glyph(ch);
    case FontType::FreeType:
        return m_font_desc.free_type.load_glyph(ch);
    default:
        std::abort();
    }
}

} // namespace LG