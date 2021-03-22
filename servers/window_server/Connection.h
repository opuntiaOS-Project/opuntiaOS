/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "WSConnection.h"
#include "WSServerDecoder.h"
#include <libfoundation/EventReceiver.h>
#include <libipc/ServerConnection.h>
#include <syscalls.h>

namespace WinServer {

class Connection : public LFoundation::EventReceiver {
public:
    static Connection& the();
    explicit Connection(int connection_fd);

    inline void listen()
    {
        m_connection_with_clients.pump_messages();
    }

    inline bool send_async_message(const Message& msg) const { return m_connection_with_clients.send_message(msg); }
    inline int alloc_connection() { return ++m_connections_number; }
    void receive_event(std::unique_ptr<LFoundation::Event> event) override;

private:
    int m_connection_fd;
    int m_connections_number { 0 };
    ServerConnection<WServerDecoder, WindowClientDecoder> m_connection_with_clients;
    WServerDecoder m_server_decoder;
    WindowClientDecoder m_client_decoder;
};

} // namespace WinServer