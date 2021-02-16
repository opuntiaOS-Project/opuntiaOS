/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __oneOS__IO__SHARED_BUFFER_H
#define __oneOS__IO__SHARED_BUFFER_H

#include <types.h>

int shared_buffer_init();
int shared_buffer_create(uint8_t** buffer, size_t size);
int shared_buffer_get(int id, uint8_t** buffer);
int shared_buffer_free(int id);

#endif /* __oneOS__IO__SHARED_BUFFER_H */