/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**
 * !!! ALERT !!!
 *  
 * After each push, your result pointers, you have got from get() could be broken!
 * For now, it's better to avoid using of dynamic array!
 * Need to be fixed.
 */

#ifndef _KERNEL_ALGO_DYNAMIC_ARRAY_H
#define _KERNEL_ALGO_DYNAMIC_ARRAY_H

#include <libkern/types.h>

struct dynamic_array {
    void* data;
    uint32_t capacity; /* current allocated memory */
    uint32_t size; /* number of elements in vector */
    uint32_t element_size; /* size of elements in bytes */
};
typedef struct dynamic_array dynamic_array_t;

int dynamic_array_init(dynamic_array_t* v, uint32_t element_size);
int dynamic_array_init_of_size(dynamic_array_t* v, uint32_t element_size, uint32_t capacity);
int dynamic_array_free(dynamic_array_t* v);

void* dynamic_array_get(dynamic_array_t* v, int index);
int dynamic_array_push(dynamic_array_t* v, void* element);
int dynamic_array_pop(dynamic_array_t* v);
int dynamic_array_clear(dynamic_array_t* v);

#endif // _KERNEL_ALGO_DYNAMIC_ARRAY_H