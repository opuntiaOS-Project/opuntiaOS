/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#pragma once
#include "../servers/window_server/WSConnection.h"
#include "View.h"
#include <libcxx/sys/SharedBuffer.h>
#include <libfoundation/Event.h>
#include <libfoundation/EventReceiver.h>
#include <libg/Color.h>
#include <libg/PixelBitmap.h>
#include <libg/String.h>
#include <sys/types.h>

namespace UI {

class Window : public LFoundation::EventReceiver {
public:
    Window();
    Window(uint32_t width, uint32_t height);

    int id() const { return m_id; }

    SharedBuffer<LG::Color>& buffer() { return m_buffer; }
    const SharedBuffer<LG::Color>& buffer() const { return m_buffer; }

    LG::PixelBitmap& bitmap() { return m_bitmap; }
    const LG::PixelBitmap& bitmap() const { return m_bitmap; }

    void receive_event(UniquePtr<LFoundation::Event> event) override;

    inline void set_superview(View* superview)
    {
        superview->set_window(this);
        m_superview = superview;
    }
    inline View* superview() const { return m_superview; }

    inline void set_focused_view(View* view) { m_focused_view = view; }
    inline View* focused_view() { return m_focused_view; }

    inline const LG::Rect& bounds() const { return m_bounds; }

    bool set_title(const LG::String& title);
    bool set_frame_style(const LG::Color& color);

private:
    uint32_t m_id;
    LG::Rect m_bounds;
    LG::PixelBitmap m_bitmap;
    SharedBuffer<LG::Color> m_buffer;
    View* m_superview { nullptr };
    View* m_focused_view { nullptr };
};

}