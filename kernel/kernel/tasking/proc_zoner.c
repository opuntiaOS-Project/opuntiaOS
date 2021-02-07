/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <errno.h>
#include <fs/vfs.h>
#include <log.h>
#include <mem/kmalloc.h>
#include <tasking/proc.h>

/**
 * PROC ZONING
 */

static inline bool _proc_zones_intersect(uint32_t start1, uint32_t size1, uint32_t start2, uint32_t size2)
{
    uint32_t end1 = start1 + size1 - 1;
    uint32_t end2 = start2 + size2 - 1;
    return (start1 <= start2 && start2 <= end1) || (start1 <= end2 && end2 <= end1) || (start2 <= start1 && start1 <= end2) || (start2 <= end1 && end1 <= end2);
}

static inline bool _proc_can_fixup_zone(proc_t* proc, uint32_t* start_ptr, int* len_ptr)
{
    uint32_t zones_count = proc->zones.size;

    for (uint32_t i = 0; i < zones_count; i++) {
        proc_zone_t* zone = (proc_zone_t*)dynamic_array_get(&proc->zones, i);
        if (_proc_zones_intersect(*start_ptr, *len_ptr, zone->start, zone->len)) {
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

static inline bool _proc_can_add_zone(proc_t* proc, uint32_t start, uint32_t len)
{
    uint32_t zones_count = proc->zones.size;

    for (uint32_t i = 0; i < zones_count; i++) {
        proc_zone_t* zone = (proc_zone_t*)dynamic_array_get(&proc->zones, i);
        if (_proc_zones_intersect(start, len, zone->start, zone->len)) {
            return false;
        }
    }

    return true;
}

/**
 * Inserts zone, which won't overlap with existing ones.
 */
proc_zone_t* proc_extend_zone(proc_t* proc, uint32_t start, uint32_t len)
{
    len += (start & (VMM_PAGE_SIZE - 1));
    start &= ~(VMM_PAGE_SIZE - 1);
    if (len % VMM_PAGE_SIZE) {
        len += VMM_PAGE_SIZE - (len % VMM_PAGE_SIZE);
    }

    proc_zone_t new_zone;
    new_zone.type = 0;
    new_zone.flags = ZONE_USER;

    if (_proc_can_fixup_zone(proc, &start, &len)) {
        new_zone.start = start;
        new_zone.len = len;
        if (dynamic_array_push(&proc->zones, &new_zone) != 0) {
            return 0;
        }
        return (proc_zone_t*)dynamic_array_get(&proc->zones, proc->zones.size - 1);
    }

    return 0;
}

proc_zone_t* proc_new_zone(proc_t* proc, uint32_t start, uint32_t len)
{
    len += (start & (VMM_PAGE_SIZE - 1));
    start &= ~(VMM_PAGE_SIZE - 1);
    if (len % VMM_PAGE_SIZE) {
        len += VMM_PAGE_SIZE - (len % VMM_PAGE_SIZE);
    }

    proc_zone_t new_zone;
    new_zone.start = start;
    new_zone.len = len;
    new_zone.type = 0;
    new_zone.flags = ZONE_USER;

    if (_proc_can_add_zone(proc, start, len)) {
        if (dynamic_array_push(&proc->zones, &new_zone) != 0) {
            return 0;
        }
        return (proc_zone_t*)dynamic_array_get(&proc->zones, proc->zones.size - 1);
    }

    return 0;
}

/* FIXME: Think of more efficient way */
proc_zone_t* proc_new_random_zone(proc_t* proc, uint32_t len)
{
    if (len % VMM_PAGE_SIZE) {
        len += VMM_PAGE_SIZE - (len % VMM_PAGE_SIZE);
    }

    uint32_t zones_count = proc->zones.size;

    /* Check if we can put it at the beginning */
    proc_zone_t* ret = proc_new_zone(proc, 0, len);
    if (ret) {
        return ret;
    }

    uint32_t min_start = 0xffffffff;

    for (uint32_t i = 0; i < zones_count; i++) {
        proc_zone_t* zone = (proc_zone_t*)dynamic_array_get(&proc->zones, i);
        if (_proc_can_add_zone(proc, zone->start + zone->len, len)) {
            if (min_start > zone->start + zone->len) {
                min_start = zone->start + zone->len;
            }
        }
    }

    if (min_start == 0xffffffff) {
        return 0;
    }

    return proc_new_zone(proc, min_start, len);
}

/* FIXME: Think of more efficient way */
proc_zone_t* proc_new_random_zone_backward(proc_t* proc, uint32_t len)
{
    if (len % VMM_PAGE_SIZE) {
        len += VMM_PAGE_SIZE - (len % VMM_PAGE_SIZE);
    }

    uint32_t zones_count = proc->zones.size;

    /* Check if we can put it at the end */
    proc_zone_t* ret = proc_new_zone(proc, KERNEL_BASE - len, len);
    if (ret) {
        return ret;
    }

    uint32_t max_end = 0;

    for (uint32_t i = 0; i < zones_count; i++) {
        proc_zone_t* zone = (proc_zone_t*)dynamic_array_get(&proc->zones, i);
        if (_proc_can_add_zone(proc, zone->start - len, len)) {
            if (max_end < zone->start) {
                max_end = zone->start;
            }
        }
    }

    if (max_end == 0) {
        return 0;
    }

    return proc_new_zone(proc, max_end - len, len);
}

proc_zone_t* proc_find_zone(proc_t* proc, uint32_t addr)
{
    uint32_t zones_count = proc->zones.size;

    for (uint32_t i = 0; i < zones_count; i++) {
        proc_zone_t* zone = (proc_zone_t*)dynamic_array_get(&proc->zones, i);
        if (zone->start <= addr && addr < zone->start + zone->len) {
            return zone;
        }
    }

    return 0;
}