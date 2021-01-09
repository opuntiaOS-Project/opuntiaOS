/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <algo/dynamic_array.h>
#include <utils/mem.h>
#include <drivers/x86/display.h>
#include <mem/kmalloc.h>

static inline int _dynamic_array_resize(dynamic_array_t* v, uint32_t new_capacity, uint32_t element_size)
{
    uint32_t* new_darray_area = krealloc(v->data, new_capacity * element_size);
    if (new_darray_area == 0) {
        return -1;
    }
    v->data = new_darray_area;
    v->capacity = new_capacity;
    return 0;
}

static inline int _dynamic_array_grow(dynamic_array_t* v)
{
    if (v->capacity) {
        if (_dynamic_array_resize(v, 2 * v->capacity, v->element_size) != 0) {
            return -1;
        }
    } else {
        v->data = kmalloc(v->element_size); /* allocating one entry */
        if (!v->data) {
            return -1;
        }
        v->capacity = 1;
    }
    return 0;
}

int dynamic_array_init(dynamic_array_t* v, uint32_t element_size) 
{
    v->size = 0;
    v->capacity = 0;
    v->element_size = element_size;
    return 0;
}

int dynamic_array_init_of_size(dynamic_array_t* v, uint32_t element_size, uint32_t capacity) 
{
    v->data = kmalloc(capacity * element_size);
    if (!v->data) {
        return -1;
    }

    v->size = 0;
    v->capacity = capacity;
    v->element_size = element_size;

    return 0;
}

int dynamic_array_free(dynamic_array_t* v)
{
    kfree(v->data);
    v->size = 0;
    v->capacity = 0;
    return 0;
}

void* dynamic_array_get(dynamic_array_t* v, int index)
{
    if (index >= v->size) {
        kprintf("!!! dynamic array: index out of range");
        return 0;
    }
    return (void*)v->data + index * v->element_size;
}

int dynamic_array_push(dynamic_array_t* v, void* element)
{
    if (v->size == v->capacity) {
        if (_dynamic_array_grow(v) != 0) {
            return -1;
        }
    }
    v->size++;
    void* place = dynamic_array_get(v, v->size-1);
    memcpy(place, element, v->element_size);
    return 0;
}

int dynamic_array_pop(dynamic_array_t* v)
{
    if (v->size) {
        v->size--;
        return 0;
    }
    return -1;
}

int dynamic_array_clear(dynamic_array_t* v)
{
    v->size = 0;
    return 0;
}

uint32_t dynamic_array_size(dynamic_array_t* v)
{
    return v->size;
}