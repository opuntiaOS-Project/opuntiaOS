/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <libfreetype/freetype/freetype.h>
#include <sys/types.h>

namespace LG {

struct GlyphMetrics {
    uint8_t width { 0 };
    uint8_t height { 0 };
    uint8_t top_bearing { 0 };
    uint8_t left_bearing { 0 };
    uint8_t baseline { 0 };
    uint8_t advance { 0 };
    uint16_t font_size { 0 };
};

class Font;
class Glyph {
public:
    friend class Font;
    enum class ConstDataMarker : int {}; // Pointer to the data is valid while thid glyph is valid.

    enum Type {
        None,
        PlainBitmap,
        FreeType,
    };

    Glyph() = default;
    Glyph(const void* data, const GlyphMetrics&);
    Glyph(const void* data, const GlyphMetrics&, ConstDataMarker);
    Glyph(Glyph&& gl);
    ~Glyph();

    LG::Glyph& operator=(const LG::Glyph& gl);
    LG::Glyph& operator=(LG::Glyph&& gl);

    inline Type type() const { return m_type; }

    inline size_t width() const { return m_metrics.width; }
    inline size_t height() const { return m_metrics.height; }

    inline int top() const { return m_metrics.font_size - m_metrics.top_bearing - m_metrics.baseline; }
    inline int left() const { return m_metrics.left_bearing; }

    template <typename T>
    inline const T* data() const { return (T*)m_data; }
    inline bool bit_at(int x, int y) const { return data<uint32_t>()[y] & (1 << x); }
    inline uint8_t alpha_at(int x, int y) const { return data<uint8_t>()[y * m_metrics.width + x]; }

    inline size_t advance() const { return m_metrics.advance; }

    inline bool empty() const { return m_type == Type::None; }

private:
    void* m_data { nullptr };
    Type m_type { Type::None };
    GlyphMetrics m_metrics {};

    bool m_owned_data { false };
};

inline Glyph::Glyph(const void* data, const GlyphMetrics& metrics, ConstDataMarker)
    : m_data((void*)data)
    , m_metrics(metrics)
    , m_type(Type::PlainBitmap)
{
}

inline Glyph::Glyph(const void* data, const GlyphMetrics& metrics)
    : m_metrics(metrics)
    , m_type(Type::FreeType)
    , m_owned_data(true)
{
    size_t bitmap_sz = (size_t)m_metrics.width * (size_t)m_metrics.height;
    m_data = new uint8_t[bitmap_sz];
    memcpy(m_data, data, bitmap_sz);
}

inline Glyph::Glyph(Glyph&& gl)
    : m_data(gl.m_data)
    , m_metrics(gl.m_metrics)
    , m_type(gl.m_type)
    , m_owned_data(gl.m_owned_data)
{
    gl.m_owned_data = false;
    gl.m_type = Type::None;
    gl.m_data = nullptr;
}

inline Glyph::~Glyph()
{
    if (m_owned_data) {
        free(m_data);
    }
}

inline LG::Glyph& Glyph::operator=(LG::Glyph&& gl)
{
    m_data = gl.m_data;
    m_metrics = gl.m_metrics;
    m_type = gl.m_type;
    m_owned_data = gl.m_owned_data;

    gl.m_owned_data = false;
    gl.m_type = Type::None;
    gl.m_data = nullptr;

    return *this;
}

inline LG::Glyph& Glyph::operator=(const LG::Glyph& gl)
{
    if (!gl.m_owned_data) {
        m_data = gl.m_data;
    } else {
        size_t bitmap_sz = (size_t)gl.m_metrics.width * (size_t)gl.m_metrics.height;
        m_data = new uint8_t[bitmap_sz];
        memcpy(m_data, gl.m_data, bitmap_sz);
    }
    m_metrics = gl.m_metrics;
    m_type = gl.m_type;
    m_owned_data = gl.m_owned_data;
    return *this;
}

} // namespace LG