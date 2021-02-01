/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#ifndef __oneOS__ALGO__BITMAP_H
#define __oneOS__ALGO__BITMAP_H

#include <types.h>

struct bitmap
{
  uint8_t* data;
  uint32_t len;
};
typedef struct bitmap bitmap_t;

bitmap_t bitmap_wrap(uint8_t* data, uint32_t len);
bitmap_t bitmap_allocate(uint32_t len);
int bitmap_find_space(bitmap_t bitmap, int req);
int bitmap_find_space_aligned(bitmap_t bitmap, int req, int alignment);
int bitmap_set(bitmap_t bitmap, int where);
int bitmap_unset(bitmap_t bitmap, int where);
int bitmap_set_range(bitmap_t bitmap, int start, int len);
int bitmap_unset_range(bitmap_t bitmap, int start, int len);
#endif //__oneOS__ALGO__BITMAP_H