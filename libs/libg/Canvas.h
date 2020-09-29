#pragma once

#include "Color.h"
#include "PixelBitmap.h"

namespace LG {

class Canvas {
public:
    Canvas() = default;
    Canvas(PixelBitmap bitmap);
    ~Canvas() { }

    inline int width() const { return m_width; }
    inline int height() const { return m_height; }
    inline PixelBitmap& bitmap() { return m_bitmap; }
    inline const PixelBitmap& bitmap() const { return m_bitmap; }

    void draw(int x, int y, const Canvas& canvas);

private:
    inline int offset_in_buffer(int x, int y) { return width() * y + x; }

    PixelBitmap m_bitmap {};
    int m_width { 0 };
    int m_height { 0 };
};

}