/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#ifndef __oneOS__IO__SHARED_BUFFER_H
#define __oneOS__IO__SHARED_BUFFER_H

#include <types.h>

int shared_buffer_init();
int shared_buffer_create(uint8_t** buffer, size_t size);
int shared_buffer_get(int id, uint8_t** buffer);
int shared_buffer_free(int id);

#endif /* __oneOS__IO__SHARED_BUFFER_H */