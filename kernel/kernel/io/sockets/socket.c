/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <io/sockets/socket.h>
#include <libkern/kassert.h>

socket_t socket_list[MAX_SOCKET_COUNT];
static int next_socket = 0;

static socket_t* _socket_create(int domain, int type, int protocol)
{
    socket_list[next_socket].domain = domain;
    socket_list[next_socket].type = type;
    socket_list[next_socket].protocol = protocol;
    socket_list[next_socket].buffer = ringbuffer_create_std();
    socket_list[next_socket].d_count = 1;
    return &socket_list[next_socket++];
}

int socket_create(int domain, int type, int protocol, file_descriptor_t* fd, file_ops_t* ops)
{
    fd->type = FD_TYPE_SOCKET;
    fd->sock_entry = _socket_create(domain, type, protocol);
    fd->ops = ops;
    if (!fd->sock_entry) {
        return -1;
    }
    return 0;
}

socket_t* socket_duplicate(socket_t* sock)
{
    sock->d_count++;
    return sock;
}

int socket_put(socket_t* sock)
{
    sock->d_count--;
    ASSERT(sock->d_count > 0);
    if (sock->d_count == 0) {
        ringbuffer_free(&sock->buffer);
    }
    return 0;
}