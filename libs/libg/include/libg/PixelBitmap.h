/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <cstring>
#include <libg/Color.h>
#include <libg/Rect.h>
#include <new>
#include <sys/types.h>

namespace LG {

enum PixelBitmapFormat {
    RGB,
    RGBA,
};

class PixelBitmap {
public:
    PixelBitmap() = default;
    PixelBitmap(size_t width, size_t height, PixelBitmapFormat format = RGB);
    PixelBitmap(Color* buffer, size_t width, size_t height, PixelBitmapFormat format = RGB);
    PixelBitmap(PixelBitmap& bitmap);
    PixelBitmap(PixelBitmap&& moved_bitmap) noexcept;
    ~PixelBitmap()
    {
        clear();
    }

    void clear()
    {
        if (m_should_free) {
            delete m_data;
        }
        m_data = nullptr;
        m_bounds.set_width(0);
        m_bounds.set_height(0);
    }

    PixelBitmap& operator=(const PixelBitmap& bitmap)
    {
        clear();
        m_bounds = bitmap.bounds();
        m_should_free = bitmap.m_should_free;
        m_format = bitmap.m_format;
        if (m_should_free) {
            size_t len = width() * height() * sizeof(Color);
            m_data = (Color*)malloc(len);
            memcpy((uint8_t*)m_data, (uint8_t*)bitmap.m_data, len);
        } else {
            m_data = bitmap.m_data;
        }
        return *this;
    }

    PixelBitmap& operator=(PixelBitmap&& moved_bitmap) noexcept
    {
        m_data = moved_bitmap.m_data;
        m_bounds = moved_bitmap.bounds();
        m_should_free = moved_bitmap.m_should_free;
        m_format = moved_bitmap.m_format;
        moved_bitmap.m_data = nullptr;
        moved_bitmap.bounds().set_width(0);
        moved_bitmap.bounds().set_height(0);
        moved_bitmap.m_should_free = false;
        return *this;
    }

    inline size_t width() const { return m_bounds.width(); }
    inline size_t height() const { return m_bounds.height(); }
    inline LG::Rect& bounds() { return m_bounds; }
    inline const LG::Rect& bounds() const { return m_bounds; }
    inline Color* data() const { return m_data; }
    inline Color* line(size_t i) { return m_data + i * width(); }
    inline const Color* line(size_t i) const { return m_data + i * width(); }
    inline Color* operator[](size_t i) { return line(i); }
    inline const Color* operator[](size_t i) const { return line(i); }
    void resize(size_t width, size_t height);

    inline void set_format(PixelBitmapFormat format) { m_format = format; }
    inline PixelBitmapFormat format() const { return m_format; }
    inline bool has_alpha_channel() const { return m_format == RGBA; }

private:
    Color* m_data { nullptr };
    LG::Rect m_bounds { 0, 0, 0, 0 };
    bool m_should_free { false };
    PixelBitmapFormat m_format { RGB };
};

} // namespace LG