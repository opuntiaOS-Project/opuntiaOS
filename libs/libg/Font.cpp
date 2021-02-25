/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libg/Font.h>
#include <new>
#include <string.h>
#include <syscalls.h>

namespace LG {

/* SerenityOS font header */
struct [[gnu::packed]] FontFileHeader {
    char magic[4];
    uint8_t glyph_width;
    uint8_t glyph_height;
    uint8_t type;
    uint8_t is_variable_width;
    uint8_t glyph_spacing;
    uint8_t unused[5];
    char name[64];
};

Font& Font::system_font()
{
    static Font* s_system_font_ptr;
    const static char* s_system_font_path = "/res/fonts/system.font";
    if (!s_system_font_ptr) {
        s_system_font_ptr = Font::load_from_file(s_system_font_path);
    }
    return *s_system_font_ptr;
}

Font& Font::system_bold_font()
{
    static Font* s_system_bold_font_ptr;
    const static char* s_system_bold_font_path = "/res/fonts/systembold.font";
    if (!s_system_bold_font_ptr) {
        s_system_bold_font_ptr = Font::load_from_file(s_system_bold_font_path);
    }
    return *s_system_bold_font_ptr;
}

Font::Font(uint32_t* raw_data, uint8_t* width_data, uint8_t width, uint8_t height, size_t count, bool dynamic_width)
    : m_raw_data(raw_data)
    , m_width_data(width_data)
    , m_width(width)
    , m_height(height)
    , m_count(count)
    , m_dynamic_width(dynamic_width)
{
}

Font* Font::load_from_file(const char* path)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        return nullptr;
    }

    fstat_t stat;
    fstat(fd, &stat);

    mmap_params_t mmap_params;
    mmap_params.prot = PROT_READ;
    mmap_params.flags = MAP_PRIVATE;
    mmap_params.fd = fd;
    mmap_params.size = stat.size;

    uint8_t* res = (uint8_t*)mmap(&mmap_params);
    return Font::load_from_mem(res);
}

Font* Font::load_from_mem(uint8_t* font_data)
{
    if (!font_data) {
        return nullptr;
    }

    FontFileHeader& header = *(FontFileHeader*)font_data;

    if (memcmp((uint8_t*)header.magic, (const uint8_t*)"!Fnt", 4)) {
        Dbg() << "Font unsupported\n";
        return nullptr;
    }

    size_t count = 0;
    if (header.type == 0) {
        count = 256;
    } else if (header.type == 1) {
        count = 384;
    } else {
        Dbg() << "Type unsupported\n";
        return nullptr;
    }

    size_t bytes_per_glyph = sizeof(uint32_t) * header.glyph_height;

    uint32_t* raw_data = (uint32_t*)(font_data + sizeof(FontFileHeader));
    uint8_t* width_data = nullptr;
    if (header.is_variable_width) {
        width_data = (uint8_t*)((uint8_t*)(raw_data) + count * bytes_per_glyph);
    }

    return new Font(raw_data, width_data, header.glyph_width, header.glyph_height, count, header.is_variable_width);
}

GlyphBitmap Font::glyph_bitmap(size_t ch) const
{
    return GlyphBitmap(&m_raw_data[ch * m_height], glyph_width(ch), m_height);
}

}