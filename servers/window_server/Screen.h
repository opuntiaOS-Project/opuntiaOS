/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include <libg/Color.h>
#include <libg/PixelBitmap.h>
#include <memory>
#include <syscalls.h>

class Screen {
public:
    static Screen& the();
    Screen();

    void swap_buffers();

    inline size_t width() { return m_bounds.width(); }
    inline size_t height() const { return m_bounds.height(); }
    inline LG::Rect& bounds() { return m_bounds; }
    inline const LG::Rect& bounds() const { return m_bounds; }
    inline uint32_t depth() const { return m_depth; }

    inline LG::PixelBitmap& write_bitmap() { return *m_write_bitmap_ptr; }
    inline const LG::PixelBitmap& write_bitmap() const { return *m_write_bitmap_ptr; }
    inline LG::PixelBitmap& display_bitmap() { return *m_display_bitmap_ptr; }
    inline const LG::PixelBitmap& display_bitmap() const { return *m_display_bitmap_ptr; }

private:
    int m_screen_fd;
    LG::Rect m_bounds;
    uint32_t m_depth;

    int m_active_buffer;

    LG::PixelBitmap m_write_bitmap;
    LG::PixelBitmap m_display_bitmap;

    std::unique_ptr<LG::PixelBitmap> m_write_bitmap_ptr { nullptr };
    std::unique_ptr<LG::PixelBitmap> m_display_bitmap_ptr { nullptr };
};