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

}