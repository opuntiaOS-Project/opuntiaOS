/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/driver_manager.h>
#include <drivers/generic/screen.h>
#include <libkern/font/font.h>
#include <libkern/types.h>
#include <mem/boot.h>
#include <mem/kmemzone.h>
#include <mem/vmm.h>
#include <platform/generic/system.h>

static int _cursor_x = 0;
static int _cursor_y = 0;
static int _fbwidth = 0;
static int _fbheight = 0;
static int _fbpixels_per_row = 0;
static int _fbscale = 1;
static uint32_t _basecolor = 0x0;
static uintptr_t _fb_paddr = 0;
static volatile uint32_t* _fb = NULL;
static kmemzone_t mapped_zone;

static void update_cursor_position(char c)
{
    _cursor_x += 8 * _fbscale;

    if (c == '\n' || (_cursor_x + 8 * _fbscale) > _fbwidth) {
        _cursor_y += 8 * _fbscale;
        _cursor_x = 0;

        if ((_cursor_y + 8 * _fbscale + 2) > _fbheight) {
            _cursor_y = 0;
        }
    }
}

static void draw_char_on_screen(char c)
{
    for (int x = 0; x < (8 * _fbscale); x++) {
        for (int y = 0; y < (8 * _fbscale); y++) {
            uint32_t ind = (_cursor_x + x) + ((_cursor_y + y) * _fbpixels_per_row);
            uint32_t clr = _basecolor;
            if (font8x8_basic[c & 0x7f][y / _fbscale] & (1 << (x / _fbscale))) {
                clr ^= 0xffffffff;
            }
            _fb[ind] = clr;
        }
    }
    system_cache_invalidate((void*)&_fb[_cursor_y * _fbpixels_per_row], (1 + 8 * _fbscale) * _fbpixels_per_row * 4);
    update_cursor_position(c);
}

int screen_setup(boot_args_t* boot_args)
{
    if (!boot_args) {
        return -1;
    }

    if (!boot_args->fb_boot_desc.vaddr) {
        return -1;
    }

    _cursor_x = 0;
    _cursor_y = 0;
    _fb = (uint32_t*)boot_args->fb_boot_desc.vaddr;
    _fb_paddr = boot_args->fb_boot_desc.paddr;
    _fbwidth = boot_args->fb_boot_desc.width;
    _fbheight = boot_args->fb_boot_desc.height;
    _fbpixels_per_row = boot_args->fb_boot_desc.pixels_per_row;
    _fbscale = 2;

    return 0;
}

int screen_put_char(char c)
{
    if (!_fb) {
        return -1;
    }

    draw_char_on_screen(c);
    return 0;
}

static inline int _screen_map_itself()
{
    size_t len = 16 << 20;
    uintptr_t paddr_offset = _fb_paddr & (VMM_PAGE_SIZE - 1);
    uintptr_t mmio_paddr = ROUND_FLOOR(_fb_paddr, VMM_PAGE_SIZE);

    mapped_zone = kmemzone_new(len);
    vmm_map_pages(mapped_zone.start, mmio_paddr, 256 * 16, MMU_FLAG_DEVICE);
    _fb = (uint32_t*)(mapped_zone.start + paddr_offset);
    return 0;
}

void screen_remap()
{
    if (!_fb) {
        return;
    }
    _screen_map_itself();
}
devman_register_driver_installation_order(screen_remap, 10);