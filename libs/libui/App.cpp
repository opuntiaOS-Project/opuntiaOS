/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libui/App.h>
#include <memory>
#include <std/Dbg.h>

namespace UI {

static App* s_the = nullptr;

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

void App::receive_event(std::unique_ptr<LFoundation::Event> event)
{
    if (event->type() == Event::Type::WindowCloseRequestEvent) {
        // TODO: Only 1 window is supported for now
        WindowCloseRequestEvent& own_event = *(WindowCloseRequestEvent*)event.get();
        auto message = DestroyWindowMessage(m_server_connection.key(), own_event.window_id());
        auto reply = m_server_connection.send_sync_message<DestroyWindowMessageReply>(message);
        m_event_loop.stop(reply->status());
    }
}

} // namespace UI