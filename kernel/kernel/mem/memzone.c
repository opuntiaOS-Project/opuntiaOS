/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <fs/vfs.h>
#include <libkern/bits/errno.h>
#include <libkern/log.h>
#include <mem/kmalloc.h>
#include <tasking/proc.h>

/**
 * PROC ZONING
 */

static inline bool _pzones_intersect(size_t start1, size_t size1, size_t start2, size_t size2)
{
    size_t end1 = start1 + size1 - 1;
    size_t end2 = start2 + size2 - 1;
    return (start1 <= start2 && start2 <= end1) || (start1 <= end2 && end2 <= end1) || (start2 <= start1 && start1 <= end2) || (start2 <= end1 && end1 <= end2);
}

static inline bool _proc_can_fixup_zone(vm_address_space_t* vm_aspace, size_t* start_ptr, int* len_ptr)
{
    size_t zones_count = vm_aspace->zones.size;

    for (size_t i = 0; i < zones_count; i++) {
        memzone_t* zone = (memzone_t*)dynarr_get(&vm_aspace->zones, i);
        if (_pzones_intersect(*start_ptr, *len_ptr, zone->start, zone->len)) {
            if (*start_ptr >= zone->start) {
                int move = (zone->start + zone->len) - (*start_ptr);
                *start_ptr += move;
                *len_ptr -= move;
            } else {
                int move = (*start_ptr + *len_ptr) - zone->start;
                *len_ptr -= move;
            }

            if (*len_ptr <= 0) {
                return false;
            }
        }
    }

    return true;
}

static inline bool _proc_can_add_zone(vm_address_space_t* vm_aspace, size_t start, size_t len)
{
    size_t zones_count = vm_aspace->zones.size;

    for (size_t i = 0; i < zones_count; i++) {
        memzone_t* zone = (memzone_t*)dynarr_get(&vm_aspace->zones, i);
        if (_pzones_intersect(start, len, zone->start, zone->len)) {
            return false;
        }
    }

    return true;
}

static inline void _proc_swap_zones(memzone_t* one, memzone_t* two)
{
    memzone_t tmp = *one;
    *one = *two;
    *two = tmp;
}

/**
 * Inserts zone, which won't overlap with existing ones.
 */
memzone_t* memzone_extend(vm_address_space_t* vm_aspace, size_t start, size_t len)
{
    len += (start & (VMM_PAGE_SIZE - 1));
    start &= ~(VMM_PAGE_SIZE - 1);
    if (len % VMM_PAGE_SIZE) {
        len += VMM_PAGE_SIZE - (len % VMM_PAGE_SIZE);
    }

    memzone_t new_zone = { 0 };
    new_zone.type = 0;
    new_zone.flags = ZONE_USER;

    if (_proc_can_fixup_zone(vm_aspace, &start, (int*)&len)) {
        new_zone.start = start;
        new_zone.len = len;
        if (!dynarr_push(&vm_aspace->zones, &new_zone)) {
            return NULL;
        }
        return (memzone_t*)dynarr_get(&vm_aspace->zones, vm_aspace->zones.size - 1);
    }

    return NULL;
}

memzone_t* memzone_new(vm_address_space_t* vm_aspace, size_t start, size_t len)
{
    len += (start & (VMM_PAGE_SIZE - 1));
    start &= ~(VMM_PAGE_SIZE - 1);
    if (len % VMM_PAGE_SIZE) {
        len += VMM_PAGE_SIZE - (len % VMM_PAGE_SIZE);
    }

    memzone_t new_zone = { 0 };
    new_zone.start = start;
    new_zone.len = len;
    new_zone.type = 0;
    new_zone.flags = ZONE_USER;
    new_zone.ops = NULL;

    if (_proc_can_add_zone(vm_aspace, start, len)) {
        if (!dynarr_push(&vm_aspace->zones, &new_zone)) {
            return NULL;
        }
        return (memzone_t*)dynarr_get(&vm_aspace->zones, vm_aspace->zones.size - 1);
    }
    return NULL;
}

// TODO: Think of more efficient way
memzone_t* memzone_new_random(vm_address_space_t* vm_aspace, size_t len)
{
    if (len % VMM_PAGE_SIZE) {
        len += VMM_PAGE_SIZE - (len % VMM_PAGE_SIZE);
    }

    size_t zones_count = vm_aspace->zones.size;

    /* Check if we can put it at the beginning */
    memzone_t* ret = memzone_new(vm_aspace, 0, len);
    if (ret) {
        return ret;
    }

    size_t min_start = 0xffffffff;

    for (size_t i = 0; i < zones_count; i++) {
        memzone_t* zone = (memzone_t*)dynarr_get(&vm_aspace->zones, i);
        if (_proc_can_add_zone(vm_aspace, zone->start + zone->len, len)) {
            if (min_start > zone->start + zone->len) {
                min_start = zone->start + zone->len;
            }
        }
    }

    if (min_start == 0xffffffff) {
        return NULL;
    }

    return memzone_new(vm_aspace, min_start, len);
}

// TODO: Think of more efficient way
memzone_t* memzone_new_random_backward(vm_address_space_t* vm_aspace, size_t len)
{
    if (len % VMM_PAGE_SIZE) {
        len += VMM_PAGE_SIZE - (len % VMM_PAGE_SIZE);
    }

    size_t zones_count = vm_aspace->zones.size;

    /* Check if we can put it at the end */
    memzone_t* ret = memzone_new(vm_aspace, KERNEL_BASE - len, len);
    if (ret) {
        return ret;
    }

    size_t max_end = 0;

    for (size_t i = 0; i < zones_count; i++) {
        memzone_t* zone = (memzone_t*)dynarr_get(&vm_aspace->zones, i);
        if (_proc_can_add_zone(vm_aspace, zone->start - len, len)) {
            if (max_end < zone->start) {
                max_end = zone->start;
            }
        }
    }

    if (max_end == 0) {
        return NULL;
    }

    return memzone_new(vm_aspace, max_end - len, len);
}

memzone_t* memzone_find_no_proc(dynamic_array_t* zones, size_t addr)
{
    size_t zones_count = zones->size;

    for (size_t i = 0; i < zones_count; i++) {
        memzone_t* zone = (memzone_t*)dynarr_get(zones, i);
        if (zone->start <= addr && addr < zone->start + zone->len) {
            return zone;
        }
    }

    return NULL;
}

memzone_t* memzone_find(vm_address_space_t* vm_aspace, size_t addr)
{
    if (!vm_aspace) {
        return NULL;
    }
    return memzone_find_no_proc(&vm_aspace->zones, addr);
}

int memzone_free_no_proc(dynamic_array_t* zones, memzone_t* givzone)
{
    size_t zones_count = zones->size;

    for (size_t i = 0; i < zones_count; i++) {
        memzone_t* zone = (memzone_t*)dynarr_get(zones, i);
        if (givzone == zone) {
            _proc_swap_zones(zone, dynarr_get(zones, zones_count - 1));
            dynarr_pop(zones);
            return 0;
        }
    }

    return -EALREADY;
}

int memzone_free(vm_address_space_t* vm_aspace, memzone_t* givzone)
{
    return memzone_free_no_proc(&vm_aspace->zones, givzone);
}

int memzone_copy(vm_address_space_t* to_vm_aspace, vm_address_space_t* from_vm_aspace)
{
    for (int i = 0; i < from_vm_aspace->zones.size; i++) {
        memzone_t* zone_to_copy = (memzone_t*)dynarr_get(&from_vm_aspace->zones, i);
        if (zone_to_copy->file) {
            dentry_duplicate(zone_to_copy->file); // For the copied zone.
        }
        dynarr_push(&to_vm_aspace->zones, zone_to_copy);
    }

    return 0;
}