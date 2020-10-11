/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#pragma once
#include "../servers/window_server/WSConnection.h"
#include <libcxx/sys/SharedBuffer.h>
#include <libfoundation/Event.h>
#include <libfoundation/EventReceiver.h>
#include <libg/Color.h>
#include <sys/types.h>

namespace UI {

class Window : public LFoundation::EventReceiver {
public:
    Window();
    Window(uint32_t width, uint32_t height);

    int id() const { return m_id; }
    uint32_t width() const { return m_width; }
    uint32_t height() const { return m_height; }

    void run();
    SharedBuffer<LG::Color>& buffer() { return m_buffer; }
    const SharedBuffer<LG::Color>& buffer() const { return m_buffer; }

    void receive_event(UniquePtr<LFoundation::Event> event) override;
private:
    uint32_t m_id;
    uint32_t m_width;
    uint32_t m_height;
    SharedBuffer<LG::Color> m_buffer;
};

}