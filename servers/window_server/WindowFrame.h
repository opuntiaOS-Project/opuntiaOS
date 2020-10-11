/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#pragma once
#include "WSEvent.h"
#include <libfoundation/EventReceiver.h>
#include <libg/Context.h>

class Window;

class WindowFrame {
public:
    WindowFrame(Window& window);
    ~WindowFrame() { }

    void draw(LG::Context&);
    constexpr size_t top_border_size() const { return 20; }
    constexpr size_t bottom_border_size() const { return 2; }
    constexpr size_t left_border_size() const { return 2; }
    constexpr size_t right_border_size() const { return 2; }

    const LG::Rect bounds() const;

    void receive_mouse_event(UniquePtr<MouseEvent> event);

private:
    Window& m_window;
};
