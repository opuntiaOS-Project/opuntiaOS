/*
 * Copyright (C) 2020-2023 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_STORAGE_VIRTIO_BLOCK_H
#define _KERNEL_DRIVERS_STORAGE_VIRTIO_BLOCK_H

#include <drivers/driver_manager.h>
#include <drivers/virtio/virtio.h>
#include <libkern/types.h>
#include <mem/kmalloc.h>

#define VIRTIO_BLK_T_IN (0)
#define VIRTIO_BLK_T_OUT (1)
#define VIRTIO_BLK_T_FLUSH (4)
#define VIRTIO_BLK_T_DISCARD (11)
#define VIRTIO_BLK_T_WRITE_ZEROES (13)

// Status values
#define VIRTIO_BLK_S_OK (0)
#define VIRTIO_BLK_S_IOERR (1)
#define VIRTIO_BLK_S_UNSUPP (2)

// Feature bits
#define VIRTIO_BLK_F_SIZE_MAX (1)
#define VIRTIO_BLK_F_SEG_MAX (2)
#define VIRTIO_BLK_F_GEOMETRY (4)
#define VIRTIO_BLK_F_RO (5)
#define VIRTIO_BLK_F_BLK_SIZE (6)
#define VIRTIO_BLK_F_FLUSH (9)
#define VIRTIO_BLK_F_TOPOLOGY (10)
#define VIRTIO_BLK_F_CONFIG_WCE (11)
#define VIRTIO_BLK_F_DISCARD (13)
#define VIRTIO_BLK_F_WRITE_ZEROES (14)

struct block_dev {
    virtio_queue_desc_t queue_desc;
    void* ptr;
    uint32_t idx;
    uint32_t ack_used_idx;
    virtio_buffer_desc_t buffer_desc;
};
typedef struct block_dev block_dev_t;

typedef struct {
    uint32_t blktype;
    uint32_t reserved;
    uint64_t sector;
} block_header_t;

typedef struct {
    uint8_t status;
} block_status_t;

typedef struct {
    block_header_t header;
    block_status_t status;
    uint16_t head;
    uint16_t watcher;
} block_request_t;

#endif //_KERNEL_DRIVERS_STORAGE_VIRTIO_BLOCK_H