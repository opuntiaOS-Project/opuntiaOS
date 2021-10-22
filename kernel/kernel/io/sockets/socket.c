/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <algo/sync_ringbuffer.h>
#include <io/sockets/socket.h>
#include <libkern/kassert.h>

socket_t socket_list[MAX_SOCKET_COUNT];
static int next_socket = 0;

static socket_t* _socket_create(int domain, int type, int protocol)
{
    socket_list[next_socket].domain = domain;
    socket_list[next_socket].type = type;
    socket_list[next_socket].protocol = protocol;
    socket_list[next_socket].buffer = sync_ringbuffer_create_std();
    socket_list[next_socket].d_count = 1;
    lock_init(&socket_list[next_socket].lock);
    return &socket_list[next_socket++];
}

int socket_create(int domain, int type, int protocol, file_descriptor_t* fd, file_ops_t* ops)
{
    fd->type = FD_TYPE_SOCKET;
    fd->flags = O_RDWR;
    fd->sock_entry = _socket_create(domain, type, protocol);
    fd->ops = ops;
    if (!fd->sock_entry) {
        return -1;
    }
    return 0;
}

socket_t* socket_duplicate(socket_t* sock)
{
    lock_acquire(&sock->lock);
    sock->d_count++;
    lock_release(&sock->lock);
    return sock;
}

int socket_put(socket_t* sock)
{
    lock_acquire(&sock->lock);
    sock->d_count--;
    ASSERT(sock->d_count > 0);
    if (sock->d_count == 0) {
        sync_ringbuffer_free(&sock->buffer);
    }
    lock_release(&sock->lock);
    return 0;
}