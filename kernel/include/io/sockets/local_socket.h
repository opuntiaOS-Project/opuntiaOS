/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __oneOS__IO__LOCAL_SOCKET_H
#define __oneOS__IO__LOCAL_SOCKET_H

#include <io/sockets/socket.h>

int local_socket_create(int type, int protocol, file_descriptor_t* fd);
bool local_socket_can_read(dentry_t* dentry, uint32_t start);
int local_socket_read(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len);
bool local_socket_can_write(dentry_t* dentry, uint32_t start);
int local_socket_write(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len);

int local_socket_bind(file_descriptor_t* sock, char* name, uint32_t len);
int local_socket_connect(file_descriptor_t* sock, char* name, uint32_t len);


#endif /* __oneOS__IO__LOCAL_SOCKET_H */