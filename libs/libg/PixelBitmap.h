#pragma once

#include "Color.h"
#include <libcxx/malloc.h>
#include <libcxx/sys/types.h>

namespace LG {

class PixelBitmap {
public:
    PixelBitmap() = default;
    PixelBitmap(size_t width, size_t height);
    PixelBitmap(Color* buffer, size_t width, size_t height);
    PixelBitmap(PixelBitmap&& moved_bitmap) noexcept;
    ~PixelBitmap()
    {
        if (m_should_free) {
            free(m_data);
        }
    }

    PixelBitmap& operator=(PixelBitmap&& moved_bitmap) noexcept
    {
        m_data = moved_bitmap.m_data;
        m_width = moved_bitmap.m_width;
        m_height = moved_bitmap.m_height;
        m_should_free = moved_bitmap.m_should_free;
        return *this;
    }

    inline size_t width() const { return m_width; }
    inline size_t height() const { return m_height; }
    inline Color* data() const { return m_data; }
    inline Color* line(size_t i) { return m_data + i * width(); }
    inline const Color* line(size_t i) const { return m_data + i * width(); }
    inline Color* operator[](size_t i) { return line(i); }
    inline const Color* operator[](size_t i) const { return line(i); }

    void draw(int x, int y, const PixelBitmap& bitmap);

private:
    Color* m_data { nullptr };
    size_t m_width { 0 };
    size_t m_height { 0 };
    bool m_should_free { false };
};

}