/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <io/sockets/local_socket.h>
#include <drivers/display.h>

static file_ops_t local_socket_ops = {
    local_socket_can_read,
    local_socket_can_write,
    local_socket_read,
    local_socket_write,
    0,
    0,
    0,
    0,
    0
};


int local_socket_create(int type, int protocol, file_descriptor_t* fd)
{
    return socket_create(PF_LOCAL, type, protocol, fd, &local_socket_ops);
}

bool local_socket_can_read(dentry_t* dentry)
{
    socket_t* sock_entry = (socket_t*)dentry;
    return ringbuffer_space_to_read(&sock_entry->buffer) != 0;
}

int local_socket_read(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len)
{
    socket_t* sock_entry = (socket_t*)dentry;
    uint32_t read = ringbuffer_read(&sock_entry->buffer, buf, len);
    return !(read == len);
}

bool local_socket_can_write(dentry_t* dentry)
{
    socket_t* sock_entry = (socket_t*)dentry;
    return ringbuffer_space_to_write(&sock_entry->buffer) != 0;
}

int local_socket_write(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len)
{
    socket_t* sock_entry = (socket_t*)dentry;
    uint32_t written = ringbuffer_write(&sock_entry->buffer, buf, len);
    return !(written == len);
}
