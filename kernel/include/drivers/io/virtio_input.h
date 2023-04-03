/*
 * Copyright (C) 2020-2023 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_STORAGE_VIRTIO_INPUT_H
#define _KERNEL_DRIVERS_STORAGE_VIRTIO_INPUT_H

#include <drivers/driver_manager.h>
#include <drivers/irq/irq_api.h>
#include <drivers/virtio/virtio.h>
#include <libkern/types.h>
#include <mem/kmalloc.h>

struct virtio_input_event {
    int16_t event_type;
    int16_t code;
    int32_t value;
};
typedef struct virtio_input_event virtio_input_event_t;

struct virtio_input_dev {
    virtio_queue_desc_t event_queue_desc;
    virtio_queue_desc_t status_queue_desc;
    uint32_t status_ack_used_idx;
    uint32_t event_idx;
    uint32_t event_ack_used_idx;
    virtio_buffer_desc_t event_buffer_desc;
    irq_line_t irq_line;
};
typedef struct virtio_input_dev virtio_input_dev_t;

#endif //_KERNEL_DRIVERS_STORAGE_VIRTIO_INPUT_H