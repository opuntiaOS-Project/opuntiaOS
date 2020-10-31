/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#pragma once

#include "Color.h"
#include "Rect.h"
#include <libcxx/malloc.h>
#include <libcxx/std/Dbg.h>
#include <libcxx/sys/types.h>

namespace LG {

class PixelBitmap {
public:
    PixelBitmap() = default;
    PixelBitmap(size_t width, size_t height);
    PixelBitmap(Color* buffer, size_t width, size_t height);
    PixelBitmap(PixelBitmap& bitmap);
    PixelBitmap(PixelBitmap&& moved_bitmap) noexcept;
    ~PixelBitmap()
    {
        if (m_should_free) {
            delete m_data;
        }
    }

    PixelBitmap& operator=(PixelBitmap& bitmap)
    {
        m_bounds = bitmap.bounds();
        m_should_free = bitmap.m_should_free;
        if (m_should_free) {
            size_t len = width() * height() * sizeof(Color);
            m_data = (Color*)malloc(len);
            memcpy((uint8_t*)m_data, (uint8_t*)bitmap.m_data, len);
        } else {
            m_data = bitmap.m_data;
        }
        return *this;
    }

    inline PixelBitmap& operator=(PixelBitmap&& moved_bitmap) noexcept
    {
        m_data = moved_bitmap.m_data;
        m_bounds = moved_bitmap.bounds();
        m_should_free = moved_bitmap.m_should_free;
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

private:
    Color* m_data { nullptr };
    LG::Rect m_bounds { 0, 0, 0, 0 };
    bool m_should_free { false };
};

}