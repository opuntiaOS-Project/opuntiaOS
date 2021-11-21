/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <algo/dynamic_array.h>
#include <libkern/bits/errno.h>
#include <libkern/log.h>
#include <libkern/mem.h>
#include <mem/kmalloc.h>

static dynamic_array_bucket_t* _dynarr_bucket(dynamic_array_t* v, int index, int* index_inside)
{
    if (!v->head) {
        return NULL;
    }

    dynamic_array_bucket_t* bucket = v->head;
    int cur = bucket->capacity;
    int idx = index;
    while (cur <= index) {
        idx -= bucket->capacity;
        bucket = bucket->next;
        if (!bucket) {
            return NULL;
        }
        cur += bucket->capacity;
    }

    *index_inside = idx;
    return bucket;
}

static dynamic_array_bucket_t* _dynarr_add_bucket(dynamic_array_t* v, int bucket_size)
{
    size_t siz = sizeof(dynamic_array_bucket_t) + (bucket_size * v->element_size);
    dynamic_array_bucket_t* new_bucket = (dynamic_array_bucket_t*)kmalloc(siz);
    new_bucket->data = (((char*)new_bucket) + sizeof(dynamic_array_bucket_t));
    new_bucket->capacity = bucket_size;
    new_bucket->next = NULL;
    new_bucket->size = 0;

    if (v->tail) {
        dynamic_array_bucket_t* tail = v->tail;
        tail->next = new_bucket;
        v->tail = new_bucket;
    } else {
        v->head = v->tail = new_bucket;
    }

    return new_bucket;
}

static inline bool _dynarr_has_empty_space(dynamic_array_t* v)
{
    if (!v->tail) {
        return false;
    }

    return v->tail->size < v->tail->capacity;
}

static inline int _dynarr_grow(dynamic_array_t* v)
{
    int cap = 8;
    if (v->tail) {
        cap = v->tail->capacity * 2;
    }

    if (!_dynarr_add_bucket(v, cap)) {
        return -ENOMEM;
    }
    return 0;
}

int dynarr_init_of_size_impl(dynamic_array_t* v, size_t element_size, size_t cap)
{
    v->head = v->tail = NULL;
    v->size = 0;
    v->element_size = element_size;
    if (!_dynarr_add_bucket(v, cap)) {
        return -ENOMEM;
    }
    return 0;
}

int dynarr_free(dynamic_array_t* v)
{
    dynamic_array_bucket_t* start = v->head;
    while (start) {
        dynamic_array_bucket_t* next = start->next;
        kfree(start);
        start = next;
    }
    v->head = v->tail = NULL;
    v->size = 0;
    return 0;
}

void* dynarr_get(dynamic_array_t* v, int index)
{
    int index_inside = 0;
    dynamic_array_bucket_t* target = _dynarr_bucket(v, index, &index_inside);
    if (target->size > 50) {
        while (1) { }
    }
    if (!target) {
        log_warn("!!! dynamic array: index out of range");
        return NULL;
    }
    void* place = (void*)target->data + index_inside * v->element_size;
    return place;
}

void* dynarr_push(dynamic_array_t* v, void* element)
{
    if (!_dynarr_has_empty_space(v)) {
        _dynarr_grow(v);
    }

    int index_inside = 0;
    dynamic_array_bucket_t* target = _dynarr_bucket(v, v->size, &index_inside);
    ASSERT(target);
    void* place = (void*)target->data + index_inside * v->element_size;
    memcpy(place, element, v->element_size);
    target->size++;
    v->size++;
    return place;
}

int dynarr_pop(dynamic_array_t* v)
{
    if (likely(v->size)) {
        int index_inside = 0;
        dynamic_array_bucket_t* target = _dynarr_bucket(v, v->size - 1, &index_inside);
        ASSERT(target);
        target->size--;
        v->size--;
        return 0;
    }
    return -1;
}

int dynarr_clear(dynamic_array_t* v)
{
    dynarr_free(v);
    return 0;
}

size_t dyarr_size(dynamic_array_t* v)
{
    return v->size;
}