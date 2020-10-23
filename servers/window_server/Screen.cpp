/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "Screen.h"
#include "Compositor.h"
#include <cstring.h>
#include <std/Utility.h>

static Screen* s_the;

Screen& Screen::the()
{
    return *s_the;
}

Screen::Screen()
    : m_bounds(0, 0, 1024, 768)
    , m_depth(4)
    , m_write_bitmap()
    , m_display_bitmap()
{
    s_the = this;
    m_screen_fd = open("/dev/bga", 0);
    mmap_params_t mp;
    mp.flags = MAP_SHARED;
    mp.fd = m_screen_fd;
    mp.size = 1; // Ignored in kernel mapping bga file
    mp.prot = PROT_READ | PROT_WRITE; // Ignored in kernel mapping bga file

    size_t screen_buffer_size = width() * height() * depth();
    LG::Color* first_buffer = (LG::Color*)mmap(&mp);
    LG::Color* second_buffer = (LG::Color*)((uint8_t*)first_buffer + screen_buffer_size);

    m_display_bitmap = LG::PixelBitmap(first_buffer, width(), height());
    m_write_bitmap = LG::PixelBitmap(second_buffer, width(), height());

    m_display_bitmap_ptr = &m_display_bitmap;
    m_write_bitmap_ptr = &m_write_bitmap;

    m_active_buffer = 0;
}

void Screen::swap_buffers()
{
    m_write_bitmap_ptr.swap(m_display_bitmap_ptr);
    m_active_buffer ^= 1;
    ioctl(m_screen_fd, BGA_SWAP_BUFFERS, m_active_buffer);
}
