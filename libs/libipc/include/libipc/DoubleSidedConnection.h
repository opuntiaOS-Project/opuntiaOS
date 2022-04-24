/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

namespace LIPC {

class DoubleSidedConnection {
public:
    DoubleSidedConnection(int server_to_clients_fd, int clients_to_server_fd)
        : m_clients_to_server_fd(clients_to_server_fd)
        , m_server_to_clients_fd(server_to_clients_fd)
    {
    }

    ~DoubleSidedConnection() = default;

    inline int c2s_fd() const { return m_clients_to_server_fd; }
    inline int s2c_fd() const { return m_server_to_clients_fd; }

private:
    int m_clients_to_server_fd;
    int m_server_to_clients_fd;
};

}