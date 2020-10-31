/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "PixelBitmap.h"
#include <std/Dbg.h>

namespace LG {

PixelBitmap::PixelBitmap(Color* buffer, size_t width, size_t height)
    : m_data(buffer)
    , m_bounds(0, 0, width, height)
    , m_should_free(false)
{
}

PixelBitmap::PixelBitmap(size_t width, size_t height)
    : m_bounds(0, 0, width, height)
    , m_data((Color*)malloc(sizeof(Color) * width * height))
    , m_should_free(true)
{
}

// FIXME: Possible mem leak.
PixelBitmap::PixelBitmap(PixelBitmap& bitmap)
    : m_data(nullptr)
    , m_bounds(bitmap.m_bounds)
    , m_should_free(bitmap.m_should_free)
{
    if (m_should_free) {
        size_t len = width() * height() * sizeof(Color);
        m_data = (Color*)malloc(len);
        memcpy((uint8_t*)m_data, (uint8_t*)bitmap.m_data, len);
    } else {
        m_data = bitmap.m_data;
    }
}

PixelBitmap::PixelBitmap(PixelBitmap&& moved_bitmap)
    : m_data(moved_bitmap.m_data)
    , m_bounds(moved_bitmap.m_bounds)
    , m_should_free(moved_bitmap.m_should_free)
{
    moved_bitmap.m_data = nullptr;
    moved_bitmap.bounds().set_width(0);
    moved_bitmap.bounds().set_height(0);
    moved_bitmap.m_should_free = false;
}

void PixelBitmap::resize(size_t width, size_t height)
{
    if (m_should_free) {
        free(m_data);
    }

    bounds().set_width(width);
    bounds().set_height(height);
    m_data = (Color*)malloc(sizeof(Color) * width * height);
    m_should_free = true;
}

}