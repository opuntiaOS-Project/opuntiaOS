/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libg/PixelBitmap.h>
#include <std/Dbg.h>

namespace LG {

PixelBitmap::PixelBitmap(Color* buffer, size_t width, size_t height, PixelBitmapFormat format)
    : m_data(buffer)
    , m_bounds(0, 0, width, height)
    , m_should_free(false)
    , m_format(format)
{
}

PixelBitmap::PixelBitmap(size_t width, size_t height, PixelBitmapFormat format)
    : m_bounds(0, 0, width, height)
    , m_data((Color*)malloc(sizeof(Color) * width * height))
    , m_should_free(true)
    , m_format(format)
{
}

// FIXME: Possible mem leak.
PixelBitmap::PixelBitmap(PixelBitmap& bitmap)
    : m_data(nullptr)
    , m_bounds(bitmap.m_bounds)
    , m_should_free(bitmap.m_should_free)
    , m_format(bitmap.m_format)
{
    if (m_should_free) {
        size_t len = width() * height() * sizeof(Color);
        m_data = (Color*)malloc(len);
        memcpy((uint8_t*)m_data, (uint8_t*)bitmap.m_data, len);
    } else {
        m_data = bitmap.m_data;
    }
}

PixelBitmap::PixelBitmap(PixelBitmap&& moved_bitmap) noexcept
    : m_data(moved_bitmap.m_data)
    , m_bounds(moved_bitmap.m_bounds)
    , m_should_free(moved_bitmap.m_should_free)
    , m_format(moved_bitmap.m_format)
{
    moved_bitmap.m_data = nullptr;
    moved_bitmap.bounds().set_width(0);
    moved_bitmap.bounds().set_height(0);
    moved_bitmap.m_should_free = false;
}

void PixelBitmap::resize(size_t width, size_t height)
{
    clear();
    bounds().set_width(width);
    bounds().set_height(height);
    m_data = (Color*)malloc(sizeof(Color) * width * height);
    m_should_free = true;
}

} // namespace LG