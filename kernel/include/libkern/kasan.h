/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_LIBKERN_KASAN_H
#define _KERNEL_LIBKERN_KASAN_H

#ifdef KASAN_ENABLED

#include <libkern/c_attrs.h>
#include <libkern/log.h>
#include <libkern/types.h>

#define KASAN_FREED_OBJECT (0xfb)
#define KASAN_KMALLOC_REDZONE (0xfe)
#define KASAN_FREED_PAGE (0xff)

void kasan_init(uintptr_t shadow_base, size_t shadow_size);
bool kasan_is_enabled();
void kasan_enable();
void kasan_disable();

uintptr_t kasan_mem_to_shadow(uintptr_t addr);
int kasan_poison(uintptr_t addr, size_t size, int value);
int kasan_unpoison(uintptr_t addr, size_t size, int value);
int kasan_poison_kmalloc(uintptr_t addr, size_t size);

#endif

#endif // _KERNEL_LIBKERN_KASAN_H
