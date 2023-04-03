/*
 * Copyright (C) 2020-2023 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_GRAPHICS_VIRTIO_GPU_H
#define _KERNEL_DRIVERS_GRAPHICS_VIRTIO_GPU_H

#include <drivers/driver_manager.h>
#include <drivers/virtio/virtio.h>
#include <libkern/types.h>
#include <mem/kmalloc.h>

enum CtrlType {
    // 2d commands
    CmdGetDisplayInfo = 0x0100,
    CmdResourceCreate2d,
    CmdResourceUref,
    CmdSetScanout,
    CmdResourceFlush,
    CmdTransferToHost2d,
    CmdResourceAttachBacking,
    CmdResourceDetachBacking,
    CmdGetCapsetInfo,
    CmdGetCapset,
    CmdGetEdid,
    // cursor commands
    CmdUpdateCursor = 0x0300,
    CmdMoveCursor,
    // success responses
    RespOkNoData = 0x1100,
    RespOkDisplayInfo,
    RespOkCapsetInfo,
    RespOkCapset,
    RespOkEdid,
    // error responses
    RespErrUnspec = 0x1200,
    RespErrOutOfMemory,
    RespErrInvalidScanoutId,
    RespErrInvalidResourceId,
    RespErrInvalidContextId,
    RespErrInvalidParameter,
};

struct gpu_dev {
    virtio_queue_desc_t queue_desc;
    volatile virtio_mmio_registers_t* registers;
    uint32_t idx;
    uint32_t ack_used_idx;
    virtio_buffer_desc_t fb_desc;
    size_t width;
    size_t height;
    int current_fb;
};
typedef struct gpu_dev gpu_dev_t;

struct gpu_ctrl_header {
    uint32_t ctrl_type;
    uint32_t flags;
    uint64_t fence_id;
    uint32_t ctx_id;
    uint32_t padding;
};
typedef struct gpu_ctrl_header gpu_ctrl_header_t;

struct gpu_rect {
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
};
typedef struct gpu_rect gpu_rect_t;

struct gpu_display_one {
    gpu_rect_t r;
    uint32_t enabled;
    uint32_t flags;
};
typedef struct gpu_display_one gpu_display_one_t;

#define MAX_SCANOUTS (16)
typedef struct gpu_resp_display_info {
    gpu_ctrl_header_t hdr;
    gpu_display_one_t pmodes[MAX_SCANOUTS];
} gpu_resp_display_info_t;

typedef struct gpu_get_edid {
    gpu_ctrl_header_t hdr;
    uint32_t scanout;
    uint32_t padding;
} gpu_get_edid_t;

typedef struct gpu_resp_edid {
    gpu_ctrl_header_t hdr;
    uint32_t size;
    uint32_t padding;
    uint8_t edid[1024];
} gpu_resp_edid_t;

typedef enum gpu_formats {
    GPU_FORMAT_B8G8R8A8Unorm = 1,
    GPU_FORMAT_B8G8R8X8Unorm = 2,
    GPU_FORMAT_A8R8G8B8Unorm = 3,
    GPU_FORMAT_X8R8G8B8Unorm = 4,
    GPU_FORMAT_R8G8B8A8Unorm = 67,
    GPU_FORMAT_X8B8G8R8Unorm = 68,
    GPU_FORMAT_A8B8G8R8Unorm = 121,
    GPU_FORMAT_R8G8B8X8Unorm = 134,
} gpu_formats_t;

typedef struct gpu_resource_create_2d {
    gpu_ctrl_header_t hdr;
    uint32_t resource_id;
    gpu_formats_t format;
    uint32_t width;
    uint32_t height;
} gpu_resource_create_2d_t;

typedef struct gpu_resource_unref {
    gpu_ctrl_header_t hdr;
    uint32_t resource_id;
    uint32_t padding;
} gpu_resource_unref_t;

typedef struct gpu_set_scanout {
    gpu_ctrl_header_t hdr;
    gpu_rect_t r;
    uint32_t scanout_id;
    uint32_t resource_id;
} gpu_set_scanout_t;

typedef struct gpu_resource_flush {
    gpu_ctrl_header_t hdr;
    gpu_rect_t r;
    uint32_t resource_id;
    uint32_t padding;
} gpu_resource_flush_t;

typedef struct gpu_transfer_to_host_2d {
    gpu_ctrl_header_t hdr;
    gpu_rect_t r;
    uint64_t offset;
    uint32_t resource_id;
    uint32_t padding;
} gpu_transfer_to_host_2d_t;

typedef struct gpu_attach_backing {
    gpu_ctrl_header_t hdr;
    uint32_t resource_id;
    uint32_t nr_entries;
} gpu_attach_backing_t;

typedef struct gpu_mem_entry {
    uint64_t addr;
    uint32_t length;
    uint32_t padding;
} gpu_mem_entry_t;

typedef struct gpu_detach_backing {
    gpu_ctrl_header_t hdr;
    uint32_t resource_id;
    uint32_t padding;
} gpu_detach_backing_t;

typedef struct gpu_cursor_pos {
    uint32_t scanout_id;
    uint32_t x;
    uint32_t y;
    uint32_t padding;
} gpu_cursor_pos_t;

typedef struct gpu_update_cursor {
    gpu_ctrl_header_t hdr;
    gpu_cursor_pos_t pos;
    uint32_t resource_id;
    uint32_t hot_x;
    uint32_t hot_y;
    uint32_t padding;
} gpu_update_cursor_t;

typedef struct gpu_request {
    void* request;
    void* response;
    void* alloc_start;
} gpu_request_t;

#endif //_KERNEL_DRIVERS_GRAPHICS_VIRTIO_GPU_H