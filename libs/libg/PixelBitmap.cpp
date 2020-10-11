/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

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

}