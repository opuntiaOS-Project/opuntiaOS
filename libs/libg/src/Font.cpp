/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
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
    const int buf_size = 40;
    char buf[buf_size];
    static Font* s_system_font_ptr[SystemMaxSize + 1];
    const static char* s_system_font_path = "/res/fonts/system.font/%d/regular.font";
    snprintf(buf, buf_size, s_system_font_path, size);
    if (!s_system_font_ptr[size]) {
        s_system_font_ptr[size] = Font::load_from_file(buf);
    }
    return *s_system_font_ptr[size];
}

Font& Font::system_bold_font(int size)
{
    const int buf_size = 40;
    char buf[buf_size];
    static Font* s_system_bold_font_ptr[SystemMaxSize + 1];
    const static char* s_system_bold_font_path = "/res/fonts/system.font/%d/bold.font";
    snprintf(buf, buf_size, s_system_bold_font_path, size);
    if (!s_system_bold_font_ptr[size]) {
        s_system_bold_font_ptr[size] = Font::load_from_file(buf);
    }
    return *s_system_bold_font_ptr[size];
}

Font::Font(uint32_t* raw_data, uint8_t* width_data, uint8_t width, uint8_t height, size_t count, bool dynamic_width, uint8_t glyph_spacing)
    : m_raw_data(raw_data)
    , m_width_data(width_data)
    , m_width(width)
    , m_height(height)
    , m_count(count)
    , m_dynamic_width(dynamic_width)
    , m_spacing(glyph_spacing)
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

    uint8_t* ptr = (uint8_t*)mmap(NULL, stat.size, PROT_READ, MAP_PRIVATE, fd, 0);
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

    return new Font(raw_data, width_data, header.glyph_width, header.glyph_height, count, header.is_variable_width, header.glyph_spacing);
}

GlyphBitmap Font::glyph_bitmap(size_t ch) const
{
    return GlyphBitmap(&m_raw_data[ch * m_height], glyph_width(ch), m_height);
}

} // namespace LG