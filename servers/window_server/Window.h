/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#pragma once
#include "WSConnection.h"
#include <libg/PixelBitmap.h>
#include <libg/Rect.h>
#include <std/Utility.h>
#include <std/WeakPtr.h>
#include <sys/SharedBuffer.h>
#include <sys/types.h>

class Window : public Weakable<Window> {
public:
    Window(int id, const CreateWindowMessage& msg);

    void set_buffer(int buffer_id);

    inline int id() const { return m_id; }
    inline int x() const { return m_bounds.x() + 500 * m_id; } // REMOVE
    inline int y() const { return m_bounds.y() + 200 * m_id; } // REMOVE
    inline size_t width() const { return m_bounds.width(); } // REMOVE
    inline size_t height() const { return m_bounds.height(); } // REMOVE

    inline SharedBuffer<LG::Color>& buffer() { return m_buffer; }
    inline const LG::PixelBitmap& bitmap() const { return m_bitmap; }

    inline LG::Rect& bounds() { return m_bounds; }
    inline const LG::Rect& bounds() const { return m_bounds; }

private:
    int m_id { -1 };
    LG::Rect m_bounds;
    LG::PixelBitmap m_bitmap;
    SharedBuffer<LG::Color> m_buffer;
};
