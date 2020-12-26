/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "App.h"
#include <std/Dbg.h>

namespace UI {

static App* s_the = 0;

App& App::the()
{
    return *s_the;
}

App::App()
    : m_event_loop()
    , m_server_connection(socket(PF_LOCAL, 0, 0))
{
    s_the = this;
}

void App::receive_event(UniquePtr<LFoundation::Event> event)
{
    if (event->type() == Event::Type::WindowCloseRequestEvent) {
        // TODO: Only 1 window is supported for now
        WindowCloseRequestEvent& own_event = *(WindowCloseRequestEvent*)event.get();
        auto message = DestroyWindowMessage(m_server_connection.key(), own_event.window_id());
        auto reply = m_server_connection.send_sync_message<DestroyWindowMessageReply>(message);
        m_event_loop.stop(reply->status());
    }
}

}