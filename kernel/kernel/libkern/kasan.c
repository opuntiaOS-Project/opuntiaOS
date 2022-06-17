/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifdef KASAN_ENABLED

#include <libkern/kasan.h>
#include <libkern/kassert.h>
#include <libkern/libkern.h>
#include <libkern/types.h>
#include <mem/vmm.h>
#include <tasking/dump.h>
#include <tasking/tasking.h>

static bool kasan_global_enabled = false;
static uintptr_t kasan_shadow_memory_base = 0x0;
static size_t kasan_shadow_memory_size = 0x0;

#define KASAN_RETIP (uintptr_t)(__builtin_return_address(0))
#define KASAN_ASSERT(x) \
    kasan_disable();    \
    ASSERT(x);          \
    kasan_enable();

// This prevents compilers to issue asan checks.
#define KASAN_IMPL_FUNC(sym) typeof(sym) __attribute__((no_sanitize("address"))) sym

static bool kasan_can_serve_addr(uintptr_t addr)
{
    return KERNEL_BASE <= addr && addr < KERNEL_PADDR_BASE;
}

bool kasan_is_enabled()
{
    return kasan_global_enabled && THIS_CPU->kasan_depth_counter == 0;
}
KASAN_IMPL_FUNC(kasan_is_enabled);

void kasan_enable()
{
    THIS_CPU->kasan_depth_counter--;
}
KASAN_IMPL_FUNC(kasan_is_enabled);

void kasan_disable()
{
    THIS_CPU->kasan_depth_counter++;
}
KASAN_IMPL_FUNC(kasan_is_enabled);

uintptr_t kasan_mem_to_shadow(uintptr_t addr)
{
    KASAN_ASSERT(addr >= KERNEL_BASE);
    uintptr_t shadow_addr = (addr - KERNEL_BASE) / 8 + kasan_shadow_memory_base;
    KASAN_ASSERT(shadow_addr < kasan_shadow_memory_base + kasan_shadow_memory_size);
    return shadow_addr;
}
KASAN_IMPL_FUNC(kasan_mem_to_shadow);

void kasan_dump_shadow_area(uintptr_t addr, size_t size)
{
    const int bytes_in_row = 16;
    uintptr_t shadow_start = kasan_mem_to_shadow(addr);
    uintptr_t shadow_end = kasan_mem_to_shadow(addr + size - 1);
    log_not_formatted("KASAN shadow: %zx - %zx\n", shadow_start, shadow_end);

    uintptr_t start_addr = ROUND_FLOOR(shadow_start - bytes_in_row - 1, bytes_in_row);
    uintptr_t end_addr = ROUND_CEIL(shadow_end + bytes_in_row + 1, bytes_in_row);
    for (uintptr_t ad = start_addr; ad < end_addr; ad += bytes_in_row) {
        log_not_formatted("%zx: ", ad);
        for (int byte = 0; byte < bytes_in_row; byte++) {
            char* c8 = (char*)(ad + byte);
            log_not_formatted("%x", *c8);
            if (ad + byte == shadow_start) {
                log_not_formatted("[");
            } else if (ad + byte - 1 == shadow_end) {
                log_not_formatted("]");
            } else {
                log_not_formatted(" ");
            }
        }
        log_not_formatted("\n");
    }
}
KASAN_IMPL_FUNC(kasan_dump_shadow_area);

void kasan_dump(uintptr_t addr, size_t size, bool write, uintptr_t retip)
{
    kasan_enable();
    log_not_formatted("======== KASAN ERROR ========\n");
    log_not_formatted("KASAN INFO: at %zx of size: %zx, is on write: %d\n", addr, size, write);
    log_not_formatted("KASAN LoC:  at %zx\n", retip);
    kasan_dump_shadow_area(addr, size);
    dump_kernel(NULL);
    log_not_formatted("=============================\n");
    kasan_disable();
}
KASAN_IMPL_FUNC(kasan_dump);

void kasan_init(uintptr_t shadow_base, size_t shadow_size)
{
    kasan_shadow_memory_base = shadow_base;
    kasan_shadow_memory_size = shadow_size;
    log("KASAN: Enabling, this might take a while...");
    memset((void*)kasan_shadow_memory_base, 0, kasan_shadow_memory_size);
    THIS_CPU->kasan_depth_counter = 0;
    kasan_global_enabled = true;
}
KASAN_IMPL_FUNC(kasan_init);

static bool mem_is_zero_bytes(uintptr_t addr, size_t size)
{
    char* ch8 = (char*)addr;
    for (int i = 0; i < size; i++) {
        if (ch8[i] != 0x0) {
            return false;
        }
    }
    return true;
}
KASAN_IMPL_FUNC(mem_is_zero_bytes);

static bool mem_is_zero(uintptr_t addr, size_t size)
{
    if (size < 16) {
        return mem_is_zero_bytes(addr, size);
    }

    size_t aligment = (8 - (addr % 8)) % 8;
    if (!mem_is_zero_bytes(addr, aligment)) {
        return false;
    }

    addr = ROUND_CEIL(addr, 8);
    size -= aligment;

    uint64_t* addr_u64 = (uint64_t*)addr;
    size_t words = size / 8;
    for (size_t i = 0; i < words; i++) {
        if (addr_u64[i] != 0x0) {
            return false;
        }
    }

    size_t ending = size - words * 8;
    if (!mem_is_zero_bytes((uintptr_t)&addr_u64[words], ending)) {
        return false;
    }
    return true;
}
KASAN_IMPL_FUNC(mem_is_zero);

bool kasan_check_byte(uintptr_t addr)
{
    char* shadow_byte = (char*)kasan_mem_to_shadow(addr);
    if (*shadow_byte == 0) {
        return true;
    }
    if (*shadow_byte >= 8) {
        return false;
    }
    return addr % 8 < *shadow_byte;
}
KASAN_IMPL_FUNC(kasan_check_byte);

bool kasan_check_shadow(uintptr_t addr, size_t size, bool write)
{
    if (size < 8) {
        for (int i = 0; i < size; i++) {
            if (!kasan_check_byte(addr + i)) {
                return false;
            }
        }
        return true;
    }

    uintptr_t addr_end = addr + size - 1;
    uintptr_t shadow_start = kasan_mem_to_shadow(addr);
    size_t words = size / 8;
    if (!mem_is_zero(shadow_start, words)) {
        return false;
    }

    char* shadow_byte = (char*)kasan_mem_to_shadow(addr_end);
    if (*shadow_byte == 0) {
        return true;
    }
    if (*shadow_byte >= 8) {
        return false;
    }

    int rembytes = addr_end - ROUND_CEIL(addr + words * 8, 8);
    if (!rembytes) {
        return true;
    }

    return rembytes < *shadow_byte;
}
KASAN_IMPL_FUNC(kasan_check_shadow);

bool kasan_check_range(uintptr_t addr, size_t size, bool write, uintptr_t retip)
{
    if (!kasan_is_enabled()) {
        return true;
    }

    if (!size) {
        return true;
    }

    if (!kasan_can_serve_addr(addr)) {
        return true;
    }

    // Handling overflow
    if (addr + size < addr) {
        kasan_dump(addr, size, write, retip);
    }

    if (!kasan_check_shadow(addr, size, write)) {
        kasan_dump(addr, size, write, retip);
    }

    return true;
}
KASAN_IMPL_FUNC(kasan_check_range);

int kasan_poison_kmalloc(uintptr_t addr, size_t size)
{
    if (!kasan_is_enabled()) {
        return -1;
    }

    int fullbytes = (size / 8) * 8;
    if (fullbytes) {
        kasan_poison(addr, fullbytes, 0x0);
    }

    uintptr_t shadow_byte = kasan_mem_to_shadow(addr + size - 1);
    int rembytes = size - fullbytes;
    if (rembytes) {
        *((char*)shadow_byte) = rembytes;
    }
    return 0;
}
KASAN_IMPL_FUNC(kasan_poison_kmalloc);

static void* kasan_memset(void* dest, uint8_t fll, size_t nbytes)
{
    kasan_disable();
    void* res = memset(dest, fll, nbytes);
    kasan_enable();
    return res;
}
KASAN_IMPL_FUNC(kasan_memset);

int kasan_poison(uintptr_t addr, size_t size, int value)
{
    KASAN_ASSERT(addr % 8 == 0 && size % 8 == 0);
    if (!kasan_is_enabled()) {
        return -1;
    }

    uintptr_t shadow_start = kasan_mem_to_shadow(addr);
    kasan_memset((void*)shadow_start, value, size / 8);
    return 0;
}
KASAN_IMPL_FUNC(kasan_poison);

int kasan_unpoison(uintptr_t addr, size_t size, int value)
{
    KASAN_ASSERT(addr % 8 == 0 && size % 8 == 0);
    if (!kasan_is_enabled()) {
        return -1;
    }

    uintptr_t shadow_start = kasan_mem_to_shadow(addr);
    kasan_memset((void*)shadow_start, value, size / 8);
    return 0;
}
KASAN_IMPL_FUNC(kasan_unpoison);

#define GEN_KASAN_FUNCS(size)                                                                               \
    void __asan_load##size(uintptr_t addr) { kasan_check_range(addr, size, false, KASAN_RETIP); }           \
    void __asan_load##size##_noabort(uintptr_t addr) { kasan_check_range(addr, size, false, KASAN_RETIP); } \
    void __asan_store##size(uintptr_t addr) { kasan_check_range(addr, size, true, KASAN_RETIP); }           \
    void __asan_store##size##_noabort(uintptr_t addr) { kasan_check_range(addr, size, true, KASAN_RETIP); }

GEN_KASAN_FUNCS(1)
GEN_KASAN_FUNCS(2)
GEN_KASAN_FUNCS(4)
GEN_KASAN_FUNCS(8)
GEN_KASAN_FUNCS(16)
#undef GEN_KASAN_FUNCS

void __asan_loadN(unsigned long addr, size_t size)
{
    kasan_check_range(addr, size, false, KASAN_RETIP);
}

void __asan_loadN_noabort(unsigned long addr, size_t size)
{
    kasan_check_range(addr, size, false, KASAN_RETIP);
}

void __asan_storeN(unsigned long addr, size_t size)
{
    kasan_check_range(addr, size, false, KASAN_RETIP);
}

void __asan_storeN_noabort(unsigned long addr, size_t size)
{
    kasan_check_range(addr, size, false, KASAN_RETIP);
}

void __asan_handle_no_return() { }

#endif