#include "PixelBitmap.h"
#include <libcxx/syscalls.h>

namespace LG {

PixelBitmap::PixelBitmap(Color* buffer, size_t width, size_t height)
    : m_data(buffer)
    , m_width(width)
    , m_height(height)
    , m_should_free(false)
{
}

PixelBitmap::PixelBitmap(size_t width, size_t height)
    : m_width(width)
    , m_height(height)
    , m_data((Color*)malloc(sizeof(Color) * m_width * m_height))
    , m_should_free(true)
{
}

PixelBitmap::PixelBitmap(PixelBitmap&& moved_bitmap)
    : m_data(moved_bitmap.m_data)
    , m_width(moved_bitmap.m_width)
    , m_height(moved_bitmap.m_height)
    , m_should_free(moved_bitmap.m_should_free)
{
}

void PixelBitmap::draw(int x, int y, const PixelBitmap& bitmap)
{
    for (size_t i = 0; i < bitmap.height(); i++) {
        for (size_t j = 0; j < bitmap.width(); j++) {
            int y_pos = y + i;
            int x_pos = x + j;
            if (x_pos < width() && y_pos < height()) {
                (*this)[y_pos][x_pos] = bitmap[i][j];
            }
        }
    }
}

}