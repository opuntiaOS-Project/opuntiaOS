/*
 * Copyright (C) 2020-2023 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/devtree.h>
#include <drivers/graphics/virtio_gpu.h>
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

// #define DEBUG_VIRTIO_GPU

static int gpus_count = 0;
static gpu_dev_t gpu_dev;

#define ADD_TO_QUEUE(dev, wt)                     \
    dev.queue_desc.descs->entities[dev.idx] = wt; \
    dev.idx = (dev.idx + 1) % VIRTIO_RING_SIZE

#define SET_AS_AVAIL(dev, hd)                                                      \
    dev.queue_desc.avail->ring[dev.queue_desc.avail->idx % VIRTIO_RING_SIZE] = hd; \
    dev.queue_desc.avail->idx++

#define PUSH_REQUEST(req, resp)                                 \
    virtio_alloc_result_t alloc_result;                         \
    int err = virtio_alloc_request(&req, &resp, &alloc_result); \
    if (err) {                                                  \
        log_warn("Error in virtio_alloc_request");              \
        return err;                                             \
    }                                                           \
    virtio_desc_t desc_sso = {                                  \
        .addr = (uintptr_t)alloc_result.req_paddr,              \
        .len = sizeof(req),                                     \
        .flags = VIRTIO_DESC_F_NEXT,                            \
        .next = (gpu_dev.idx + 1) % VIRTIO_RING_SIZE,           \
    };                                                          \
    virtio_desc_t desc_sso_resp = {                             \
        .addr = (uintptr_t)alloc_result.resp_paddr,             \
        .len = sizeof(resp),                                    \
        .flags = VIRTIO_DESC_F_WRITE,                           \
        .next = 0,                                              \
    };                                                          \
    uint32_t head = gpu_dev.idx;                                \
    ADD_TO_QUEUE(gpu_dev, desc_sso);                            \
    ADD_TO_QUEUE(gpu_dev, desc_sso_resp);                       \
    SET_AS_AVAIL(gpu_dev, head)

#define PUSH_MEM_REQUEST(req, mem, resp)                                  \
    virtio_alloc_result_t alloc_result;                                   \
    int err = virtio_alloc_mem_request(&req, &mem, &resp, &alloc_result); \
    if (err) {                                                            \
        log_warn("Error in virtio_alloc_mem_request");                    \
        return err;                                                       \
    }                                                                     \
    virtio_desc_t desc_ab = {                                             \
        .addr = (uintptr_t)alloc_result.req_paddr,                        \
        .len = sizeof(req),                                               \
        .flags = VIRTIO_DESC_F_NEXT,                                      \
        .next = (gpu_dev.idx + 1) % VIRTIO_RING_SIZE,                     \
    };                                                                    \
    virtio_desc_t desc_ab_mementry = {                                    \
        .addr = (uintptr_t)alloc_result.mem_paddr,                        \
        .len = sizeof(mem),                                               \
        .flags = VIRTIO_DESC_F_NEXT,                                      \
        .next = (gpu_dev.idx + 2) % VIRTIO_RING_SIZE,                     \
    };                                                                    \
    virtio_desc_t desc_ab_resp = {                                        \
        .addr = (uintptr_t)alloc_result.resp_paddr,                       \
        .len = sizeof(resp),                                              \
        .flags = VIRTIO_DESC_F_WRITE,                                     \
        .next = 0,                                                        \
    };                                                                    \
    uint32_t head = gpu_dev.idx;                                          \
    ADD_TO_QUEUE(gpu_dev, desc_ab);                                       \
    ADD_TO_QUEUE(gpu_dev, desc_ab_mementry);                              \
    ADD_TO_QUEUE(gpu_dev, desc_ab_resp);                                  \
    SET_AS_AVAIL(gpu_dev, head)

static int _virtiogpu_dev_create_2d(int bind_resource_id)
{
    gpu_resource_create_2d_t req = {
        .hdr = {
            .ctrl_type = CmdResourceCreate2d,
            .flags = 0,
            .fence_id = 0,
            .ctx_id = 0,
            .padding = 0,
        },
        .resource_id = bind_resource_id,
        .format = GPU_FORMAT_R8G8B8A8Unorm,
        .width = gpu_dev.width,
        .height = gpu_dev.height,
    };
    gpu_ctrl_header_t resp = {};
    PUSH_REQUEST(req, resp);
    return 0;
}

static int _virtiogpu_dev_attach_backing(int bind_resource_id, uint64_t framebuffer_addr)
{
    gpu_attach_backing_t req = {
        .hdr = {
            .ctrl_type = CmdResourceAttachBacking,
            .flags = 0,
            .fence_id = 0,
            .ctx_id = 0,
            .padding = 0,
        },
        .resource_id = bind_resource_id,
        .nr_entries = 1,
    };
    gpu_mem_entry_t mem = {
        .addr = framebuffer_addr,
        .length = gpu_dev.width * gpu_dev.height * 4,
        .padding = 0,
    };
    gpu_ctrl_header_t resp = {};
    PUSH_MEM_REQUEST(req, mem, resp);
    return 0;
}

static int _virtiogpu_dev_set_scanout(int bind_resource_id)
{
    gpu_set_scanout_t req = {
        .hdr = {
            .ctrl_type = CmdSetScanout,
            .flags = 0,
            .fence_id = 0,
            .ctx_id = 0,
            .padding = 0,
        },
        .r = {
            .x = 0,
            .y = 0,
            .width = gpu_dev.width,
            .height = gpu_dev.height,
        },
        .resource_id = bind_resource_id,
        .scanout_id = 0,
    };
    gpu_ctrl_header_t resp = {};
    PUSH_REQUEST(req, resp);
    return 0;
}

static int _virtiogpu_dev_transfer_to_host_2d(int bind_resource_id)
{
    gpu_transfer_to_host_2d_t req = {
        .hdr = {
            .ctrl_type = CmdTransferToHost2d,
            .flags = 0,
            .fence_id = 0,
            .ctx_id = 0,
            .padding = 0,
        },
        .r = {
            .x = 0,
            .y = 0,
            .width = gpu_dev.width,
            .height = gpu_dev.height,
        },
        .offset = 0,
        .resource_id = bind_resource_id,
        .padding = 0,
    };
    gpu_ctrl_header_t resp = {};
    PUSH_REQUEST(req, resp);
    return 0;
}

static int _virtiogpu_dev_resource_flush(int bind_resource_id)
{
    gpu_resource_flush_t req = {
        .hdr = {
            .ctrl_type = CmdResourceFlush,
            .flags = 0,
            .fence_id = 0,
            .ctx_id = 0,
            .padding = 0,
        },
        .r = {
            .x = 0,
            .y = 0,
            .width = gpu_dev.width,
            .height = gpu_dev.height,
        },
        .resource_id = bind_resource_id,
        .padding = 0,
    };
    gpu_ctrl_header_t resp = {};
    PUSH_REQUEST(req, resp);
    return 0;
}

static void _virtiogpu_dev_init()
{
    _virtiogpu_dev_create_2d(1);
    _virtiogpu_dev_attach_backing(1, gpu_dev.fb_desc.paddr);

    _virtiogpu_dev_create_2d(2);
    _virtiogpu_dev_attach_backing(2, gpu_dev.fb_desc.paddr + gpu_dev.width * gpu_dev.height * 4);

    _virtiogpu_dev_set_scanout(1);
    _virtiogpu_dev_transfer_to_host_2d(1);

    _virtiogpu_dev_resource_flush(1);

    gpu_dev.registers->queue_notify = 0;
}

static int __res = 1;
static void _virtiogpu_flip_screen()
{
    _virtiogpu_dev_set_scanout(1 + gpu_dev.current_fb);
    _virtiogpu_dev_transfer_to_host_2d(1 + gpu_dev.current_fb);
    _virtiogpu_dev_resource_flush(1 + gpu_dev.current_fb);
    gpu_dev.registers->queue_notify = 0;
    gpu_dev.current_fb = 1 - gpu_dev.current_fb;
}

void virtiogpu_int_handler(irq_line_t line)
{
    while (gpu_dev.ack_used_idx != gpu_dev.queue_desc.used->idx) {
        int id = gpu_dev.queue_desc.used->ring[gpu_dev.ack_used_idx % VIRTIO_RING_SIZE].id;
        virtio_free_paddr((void*)(uintptr_t)gpu_dev.queue_desc.descs->entities[id].addr);
        gpu_dev.ack_used_idx++;
    }
}

int virtiogpu_init(device_t* dev)
{
    if (dev->device_desc.type != DEVICE_DESC_DEVTREE) {
        return -1;
    }

    // Adding a new GPU.
    gpus_count++;

    irq_register_handler(dev->device_desc.devtree.entry->irq_lane, dev->device_desc.devtree.entry->irq_priority,
        (irq_flags_t)0x0, virtiogpu_int_handler, BOOT_CPU_MASK);

    uintptr_t mmio_vaddr = dev->device_desc.devtree.entry->region_base;
    volatile virtio_mmio_registers_t* registers = (virtio_mmio_registers_t*)mmio_vaddr;
    registers->status = 0;

    uint32_t status_bits = VIRTIO_STATUS_ACK | VIRTIO_STATUS_DRIVER_OK;
    registers->status = status_bits;

    uint32_t host_feats = registers->host_features;
    registers->guest_features = host_feats;

    status_bits |= VIRTIO_STATUS_FEATS_OK;
    registers->status = status_bits;

    uint32_t read_status = registers->status;
    if (!TEST_FLAG(read_status, VIRTIO_STATUS_FEATS_OK)) {
#ifdef DEBUG_VIRTIO_GPU
        log("VIRTIO_GPU: Can't init.");
#endif
        return 1;
    }

    uint32_t qnmax = registers->queue_num_max;
    registers->queue_num = VIRTIO_RING_SIZE;
    if (VIRTIO_RING_SIZE > qnmax) {
#ifdef DEBUG_VIRTIO_GPU
        log("VIRTIO_GPU: Ring cannot be inited.");
#endif
        return 1;
    }

    virtio_queue_desc_t queue;
    int err = virtio_alloc_queue(&queue);
    if (err) {
        return err;
    }

    registers->queue_sel = 0;
    registers->guest_page_size = VMM_PAGE_SIZE;
    registers->queue_pfn = queue.paddr / VMM_PAGE_SIZE;

    status_bits |= VIRTIO_STATUS_DRIVER_OK;
    registers->status = status_bits;

    size_t fb_alloc_size = ROUND_CEIL(2 * 1024 * 768 * 4, VMM_PAGE_SIZE);
    virtio_buffer_desc_t fb_buffer;
    err = virtio_alloc_buffer(fb_alloc_size, &fb_buffer);
    if (err) {
        return err;
    }

    gpu_dev.queue_desc = queue;
    gpu_dev.registers = registers;
    gpu_dev.idx = 0;
    gpu_dev.ack_used_idx = 0;
    gpu_dev.width = 1024;
    gpu_dev.height = 768;
    gpu_dev.current_fb = 1;
    gpu_dev.fb_desc = fb_buffer;
    _virtiogpu_dev_init();
    return 0;
}

static int _virtiogpu_ioctl(file_t* file, uintptr_t cmd, uintptr_t arg)
{
    switch (cmd) {
    case BGA_GET_HEIGHT:
        return gpu_dev.height;
    case BGA_GET_WIDTH:
        return gpu_dev.width;
    case BGA_GET_SCALE:
        return 1;
    case BGA_SWAP_BUFFERS:
        _virtiogpu_flip_screen();
        return 0;
    default:
        return -EINVAL;
    }
}

static int _virtiogpu_swap_page_mode(struct memzone* zone, uintptr_t vaddr)
{
    return SWAP_NOT_ALLOWED;
}

static vm_ops_t mmap_file_vm_ops = {
    .load_page_content = NULL,
    .restore_swapped_page = NULL,
    .swap_page_mode = _virtiogpu_swap_page_mode,
};

static memzone_t* _virtiogpu_mmap(file_t* file, mmap_params_t* params)
{
    bool map_shared = ((params->flags & MAP_SHARED) > 0);

    if (!map_shared) {
        return 0;
    }

    memzone_t* zone = memzone_new_random(RUNNING_THREAD->process->address_space, gpu_dev.fb_desc.kzone.len);
    if (!zone) {
        return 0;
    }

    zone->mmu_flags |= MMU_FLAG_PERM_WRITE | MMU_FLAG_PERM_READ | MMU_FLAG_UNCACHED;
    zone->type |= ZONE_TYPE_DEVICE;
    zone->file = file_duplicate(file);
    zone->ops = &mmap_file_vm_ops;

    for (int offset = 0; offset < gpu_dev.fb_desc.kzone.len; offset += VMM_PAGE_SIZE) {
        vmm_map_page(zone->vaddr + offset, (uintptr_t)(gpu_dev.fb_desc.paddr + offset), zone->mmu_flags);
    }

    return zone;
}

static void virtiogpu_recieve_notification(uintptr_t msg, uintptr_t param)
{
    if (!gpus_count) {
        return;
    }

    if (msg == DEVMAN_NOTIFICATION_DEVFS_READY) {
        path_t vfspth;
        if (vfs_resolve_path("/dev", &vfspth) < 0) {
            kpanic("Can't init bga in /dev");
        }

        file_ops_t fops = { 0 };
        fops.ioctl = _virtiogpu_ioctl;
        fops.mmap = _virtiogpu_mmap;
        devfs_inode_t* res = devfs_register(&vfspth, MKDEV(10, 156), "bga", 3, S_IFBLK | 0777, &fops);

        path_put(&vfspth);
    }
}

void virtiogpu_set_resolution(uint32_t width, uint32_t height)
{
    ASSERT(false && "Not supported rn");
}

static driver_desc_t _virtiogpu_driver_info()
{
    driver_desc_t virtiogpu_desc = { 0 };
    virtiogpu_desc.type = DRIVER_VIDEO_DEVICE;
    virtiogpu_desc.system_funcs.init_with_dev = virtiogpu_init;
    virtiogpu_desc.system_funcs.recieve_notification = virtiogpu_recieve_notification;
    virtiogpu_desc.functions[DRIVER_VIDEO_INIT] = virtiogpu_init;
    virtiogpu_desc.functions[DRIVER_VIDEO_SET_RESOLUTION] = virtiogpu_set_resolution;
    return virtiogpu_desc;
}

void virtiogpu_install()
{
    devman_register_driver(_virtiogpu_driver_info(), "virtiogpu");
}
devman_register_driver_installation(virtiogpu_install);