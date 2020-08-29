/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#ifndef __oneOS__IO__LOCAL_SOCKET_H
#define __oneOS__IO__LOCAL_SOCKET_H

#include <io/sockets/socket.h>

int local_socket_create(int type, int protocol, file_descriptor_t* fd);
bool local_socket_can_read(dentry_t* dentry);
int local_socket_read(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len);
bool local_socket_can_write(dentry_t* dentry);
int local_socket_write(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len);


#endif /* __oneOS__IO__LOCAL_SOCKET_H */