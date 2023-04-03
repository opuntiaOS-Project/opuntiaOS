/*
 * Copyright (C) 2020-2023 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/devtree.h>
#include <drivers/io/keyboard.h>
#include <drivers/io/mouse.h>
#include <drivers/io/virtio_input.h>
#include <drivers/irq/irq_api.h>
#include <drivers/virtio/virtio.h>
#include <fs/devfs/devfs.h>
#include <fs/vfs.h>
#include <libkern/bits/errno.h>
#include <libkern/libkern.h>
#include <libkern/log.h>
#include <libkern/types.h>
#include <mem/kmalloc.h>
#include <mem/kmemzone.h>
#include <mem/vmm.h>
#include <tasking/tasking.h>

// #define DEBUG_VIRTIO_INPUT
#define EVENT_BUFFER_ELEMENTS (128)

static int input_dev_count = 0;
static virtio_input_dev_t input_dev[4];

static bool mouse_data_changed;
static mouse_packet_t mouse_data;

#define ADD_TO_QUEUE(dev, wt)                                 \
    dev.event_queue_desc.descs->entities[dev.event_idx] = wt; \
    dev.event_idx = (dev.event_idx + 1) % VIRTIO_RING_SIZE

#define SET_AS_AVAIL(dev, hd)                                                                  \
    dev.event_queue_desc.avail->ring[dev.event_queue_desc.avail->idx % VIRTIO_RING_SIZE] = hd; \
    dev.event_queue_desc.avail->idx++

static void _virtioinp_init_buffers()
{
    generic_keyboard_init();
    generic_mouse_init();
}

static void _virtioinp_parse_data(int type, int code, int value)
{
    // Mouse left button
    if (type == 1 && code == 0x110) {
        if (value) {
            mouse_data.button_states |= 0x1;
        } else {
            mouse_data.button_states &= ~(uint16_t)0x1;
        }
        mouse_data_changed = 1;
    } else if (type == 2 && code == 0x0) {
        mouse_data.x_offset += value;
        mouse_data_changed = 1;
    } else if (type == 2 && code == 0x1) {
        mouse_data.y_offset -= value;
        mouse_data_changed = 1;
    } else if (type == 1 && value == 0x0) {
        generic_emit_key_set1(code);
    }
}

static void _virtioinp_send_mouse_packet()
{
    if (!mouse_data_changed) {
        return;
    }

    generic_mouse_send_packet(&mouse_data);
    mouse_data.x_offset = 0;
    mouse_data.y_offset = 0;
    mouse_data.wheel_data = 0;
    mouse_data_changed = 0;
}

static void _virtioinp_reset_event(size_t devid, size_t id)
{
    virtio_desc_t new_desc = {
        .addr = (uintptr_t)(input_dev[devid].event_buffer_desc.paddr + sizeof(virtio_input_event_t) * id),
        .len = sizeof(virtio_input_event_t),
        .flags = VIRTIO_DESC_F_WRITE,
        .next = 0,
    };

    uint32_t head = input_dev[devid].event_idx;
    ADD_TO_QUEUE(input_dev[devid], new_desc);
    SET_AS_AVAIL(input_dev[devid], head);
}

void _virtioinp_int_handler(irq_line_t line)
{
    size_t devid = 0;
    for (int i = 0; i < input_dev_count; i++) {
        if (input_dev[i].irq_line == line) {
            devid = i;
            break;
        }
    }

    virtio_queue_desc_t* event_queue = &input_dev[devid].event_queue_desc;
    while (input_dev[devid].event_ack_used_idx != event_queue->used->idx) {
        int id = event_queue->used->ring[input_dev[devid].event_ack_used_idx % VIRTIO_RING_SIZE].id;
        uintptr_t off = event_queue->descs->entities[id].addr - input_dev[devid].event_buffer_desc.paddr;
        virtio_input_event_t* event = (virtio_input_event_t*)(input_dev[devid].event_buffer_desc.vaddr + off);
        _virtioinp_parse_data(event->event_type, event->code, event->value);
        _virtioinp_reset_event(devid, id);
        input_dev[devid].event_ack_used_idx++;
    }

    virtio_queue_desc_t* status_queue = &input_dev[devid].status_queue_desc;
    while (input_dev[devid].status_ack_used_idx != status_queue->used->idx) {
        int id = status_queue->used->ring[input_dev[devid].status_ack_used_idx % VIRTIO_RING_SIZE].id;
        uintptr_t off = status_queue->descs->entities[id].addr - input_dev[devid].event_buffer_desc.paddr;
        virtio_input_event_t* event = (virtio_input_event_t*)(input_dev[devid].event_buffer_desc.vaddr + off);
        input_dev[devid].status_ack_used_idx++;
    }

    _virtioinp_send_mouse_packet();
}

int virtioinp_init(device_t* dev)
{
    if (dev->device_desc.type != DEVICE_DESC_DEVTREE) {
        return -1;
    }

    irq_register_handler(dev->device_desc.devtree.entry->irq_lane, dev->device_desc.devtree.entry->irq_priority,
        (irq_flags_t)0x0, _virtioinp_int_handler, BOOT_CPU_MASK);

    size_t devid = input_dev_count++;
    ASSERT(devid < 4);
    if (devid == 0) {
        _virtioinp_init_buffers();
    }

    uintptr_t mmio_vaddr = dev->device_desc.devtree.entry->region_base;
    volatile virtio_mmio_registers_t* registers = (virtio_mmio_registers_t*)mmio_vaddr;
    registers->status = 0;

    uint32_t status_bits = VIRTIO_STATUS_ACK | VIRTIO_STATUS_DRIVER_OK;
    registers->status = status_bits;

    uint32_t host_feats = registers->host_features;
    host_feats &= ~((uint32_t)(VIRTIO_F_EVENT_IDX));
    registers->guest_features = host_feats;

    status_bits |= VIRTIO_STATUS_FEATS_OK;
    registers->status = status_bits;

    uint32_t read_status = registers->status;
    if (!TEST_FLAG(read_status, VIRTIO_STATUS_FEATS_OK)) {
#ifdef DEBUG_VIRTIO_INPUT
        log_warn("VIRTIO_INPUT: Can't init");
#endif
        return 1;
    }

    uint32_t qnmax = registers->queue_num_max;
    registers->queue_num = VIRTIO_RING_SIZE;
    if (VIRTIO_RING_SIZE > qnmax) {
#ifdef DEBUG_VIRTIO_INPUT
        log_warn("VIRTIO_INPUT: Ring cannot be inited");
#endif
        return 1;
    }

    size_t queue_alloc_size = ROUND_CEIL(sizeof(virtio_queue_t), VMM_PAGE_SIZE);
    size_t queue_pages_cnt = (queue_alloc_size + VMM_PAGE_SIZE - 1) / VMM_PAGE_SIZE;

    // Setting event queue.
    virtio_queue_desc_t event_queue;
    int err = virtio_alloc_queue(&event_queue);
    if (err) {
        return err;
    }

    registers->queue_sel = 0;
    registers->guest_page_size = VMM_PAGE_SIZE;
    registers->queue_pfn = event_queue.paddr / VMM_PAGE_SIZE;

    // Setting status queue.
    virtio_queue_desc_t status_queue;
    err = virtio_alloc_queue(&status_queue);
    if (err) {
        return err;
    }

    registers->queue_sel = 1;
    registers->guest_page_size = VMM_PAGE_SIZE;
    registers->queue_pfn = status_queue.paddr / VMM_PAGE_SIZE;

    status_bits |= VIRTIO_STATUS_DRIVER_OK;
    registers->status = status_bits;

    size_t event_buffer_size = sizeof(virtio_input_event_t) * EVENT_BUFFER_ELEMENTS;
    virtio_buffer_desc_t event_buffer;
    err = virtio_alloc_buffer(event_buffer_size, &event_buffer);
    if (err) {
        return err;
    }

    input_dev[devid].event_queue_desc = event_queue;
    input_dev[devid].status_queue_desc = status_queue;
    input_dev[devid].status_ack_used_idx = 0;
    input_dev[devid].event_idx = 0;
    input_dev[devid].event_ack_used_idx = 0;
    input_dev[devid].event_buffer_desc = event_buffer;
    input_dev[devid].irq_line = dev->device_desc.devtree.entry->irq_lane;

    for (int i = 0; i < EVENT_BUFFER_ELEMENTS; i++) {
        _virtioinp_reset_event(devid, i);
    }
    return 0;
}

static void virtioinp_recieve_notification(uintptr_t msg, uintptr_t param)
{
    if (!input_dev_count) {
        return;
    }

    if (msg == DEVMAN_NOTIFICATION_DEVFS_READY) {
        generic_keyboard_create_devfs();
        generic_mouse_create_devfs();
    }
}

static driver_desc_t _virtioinp_driver_info()
{
    driver_desc_t virtio_inp = { 0 };
    virtio_inp.type = DRIVER_INPUT_SYSTEMS_DEVICE;
    virtio_inp.system_funcs.init_with_dev = virtioinp_init;
    virtio_inp.system_funcs.recieve_notification = virtioinp_recieve_notification;
    virtio_inp.functions[DRIVER_INPUT_SYSTEMS_ADD_DEVICE] = virtioinp_init;
    virtio_inp.functions[DRIVER_INPUT_SYSTEMS_GET_LAST_KEY] = NULL;
    virtio_inp.functions[DRIVER_INPUT_SYSTEMS_DISCARD_LAST_KEY] = NULL;
    return virtio_inp;
}

void virtioinp_install()
{
    devman_register_driver(_virtioinp_driver_info(), "virtioinp");
}
devman_register_driver_installation(virtioinp_install);