/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#ifndef __oneOS__ALGO__DYNAMIC_ARRAY_H
#define __oneOS__ALGO__DYNAMIC_ARRAY_H

#include <types.h>

struct dynamic_array
{
  void* data;
  uint32_t capacity; /* current allocated memory */
  uint32_t size; /* number of elements in vector */
  uint32_t element_size; /* size of elements in bytes */
};
typedef struct dynamic_array dynamic_array_t;

int dynamic_array_init(dynamic_array_t* v, uint32_t element_size);
int dynamic_array_init_of_size(dynamic_array_t* v, uint32_t capacity, uint32_t element_size);

void* dynamic_array_get(dynamic_array_t* v, int index);
int dynamic_array_push(dynamic_array_t* v, void* element);
int dynamic_array_pop(dynamic_array_t* v);
int dynamic_array_clear(dynamic_array_t* v);

#endif // __oneOS__ALGO__DYNAMIC_ARRAY_H