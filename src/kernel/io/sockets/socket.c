/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <io/sockets/socket.h>

socket_t socket_list[MAX_SOCKET_COUNT];
static int next_socket = 0;

static socket_t* _socket_create(int domain, int type, int protocol)
{
    socket_list[next_socket].domain = domain;
    socket_list[next_socket].type = type;
    socket_list[next_socket].protocol = protocol;
    socket_list[next_socket].buffer = ringbuffer_create_std();
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

int socket_free(socket_t* sock)
{
    ringbuffer_free(&sock->buffer);
    return 0;
}