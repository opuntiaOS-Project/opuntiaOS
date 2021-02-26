/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_IO_SHARED_BUFFER_SHARED_BUFFER_H
#define _KERNEL_IO_SHARED_BUFFER_SHARED_BUFFER_H

#include <libkern/types.h>

int shared_buffer_init();
int shared_buffer_create(uint8_t** buffer, size_t size);
int shared_buffer_get(int id, uint8_t** buffer);
int shared_buffer_free(int id);

#endif /* _KERNEL_IO_SHARED_BUFFER_SHARED_BUFFER_H */