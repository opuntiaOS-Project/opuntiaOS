/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_ALGO_DYNAMIC_ARRAY_H
#define _KERNEL_ALGO_DYNAMIC_ARRAY_H

#include <libkern/types.h>

// TODO: Speed up bucket search using binary jumps.
struct dynamic_array_bucket {
    void* data;
    struct dynamic_array_bucket* next;
    size_t capacity;
    size_t size;
};
typedef struct dynamic_array_bucket dynamic_array_bucket_t;

struct dynamic_array {
    dynamic_array_bucket_t* head;
    dynamic_array_bucket_t* tail;
    size_t size; /* number of elements in vector */
    size_t element_size; /* size of elements in bytes */
};
typedef struct dynamic_array dynamic_array_t;

#define dynarr_init(type, v) dynarr_init_of_size_impl(v, sizeof(type), 8)
#define dynarr_init_of_size(type, v, cap) dynarr_init_of_size_impl(v, sizeof(type), cap)

int dynarr_init_of_size_impl(dynamic_array_t* v, size_t element_size, size_t capacity);
int dynarr_free(dynamic_array_t* v);

void* dynarr_get(dynamic_array_t* v, int index);
void* dynarr_push(dynamic_array_t* v, void* element);
int dynarr_pop(dynamic_array_t* v);
int dynarr_clear(dynamic_array_t* v);

#endif // _KERNEL_ALGO_DYNAMIC_ARRAY_H