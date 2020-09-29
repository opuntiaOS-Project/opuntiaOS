#include "Canvas.h"
#include <libcxx/syscalls.h>

namespace LG {

Canvas::Canvas(PixelBitmap bitmap)
    : m_bitmap(bitmap)
    , m_width(m_bitmap.width())
    , m_height(m_bitmap.height())
{
}

void Canvas::draw(int x, int y, const Canvas& canvas)
{
    for (int i = 0; i < canvas.height(); i++) {
        for (int j = 0; j < canvas.width(); j++) {
            int y_pos = y + i;
            int x_pos = x + j;
            if (x_pos < width() && y_pos < height()) {
                bitmap()[y_pos][x_pos] = canvas.bitmap()[i][j];
            }
        }
    }
}

}