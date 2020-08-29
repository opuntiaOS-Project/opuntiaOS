/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#ifndef __oneOS__IO__SOCKET_H
#define __oneOS__IO__SOCKET_H

#include <fs/vfs.h>
#include <syscall_structs.h>
#include <types.h>

#define MAX_SOCKET_COUNT 16

int socket_create(int domain, int type, int protocol, file_descriptor_t* fd, file_ops_t* ops);
int socket_free(socket_t* sock);

#endif /* __oneOS__IO__SOCKET_H */