/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _BOOT_LIBBOOT_CRYPTO_UINT2048_H
#define _BOOT_LIBBOOT_CRYPTO_UINT2048_H

#include <libboot/mem/mem.h>
#include <libboot/types.h>

#define N_UINT2048 (64)
#define BITS_IN_BASE_UINT2048 (32)
#define BASE_UINT2048 ((uint64_t)((uint64_t)1 << BITS_IN_BASE_UINT2048))
#define BASE_UINT2048U ((uint64_t)((uint64_t)1 << (uint64_t)BITS_IN_BASE_UINT2048))
typedef uint32_t uint2048_bucket_t;
struct uint2048 {
    uint2048_bucket_t bucket[N_UINT2048];
};
typedef struct uint2048 uint2048_t;

int uint2048_init(uint2048_t* d, uint32_t n);
int uint2048_init_bytes(uint2048_t* d, const char* f, size_t n);
int uint2048_init_bytes_be(uint2048_t* d, const char* f, size_t n);
int uint2048_copy(uint2048_t* dest, uint2048_t* src);

int uint2048_add(uint2048_t* a, uint2048_t* b, uint2048_t* c);
int uint2048_sub(uint2048_t* a, uint2048_t* b, uint2048_t* c);

int uint2048_shl(uint2048_t* a, int n);
int uint2048_shr(uint2048_t* a, int n);

int uint2048_mult_by_digit(uint2048_t* a, uint2048_t* b, uint2048_bucket_t un);
int uint2048_mult(uint2048_t* a, uint2048_t* b, uint2048_t* c);

int uint2048_div(uint2048_t* a, uint2048_t* b, uint2048_t* ans, uint2048_t* rem);
int uint2048_pow(uint2048_t* ua, uint2048_t* up, uint2048_t* mod, uint2048_t* ans);

bool uint2048_equal(uint2048_t* a, uint2048_t* b);
bool uint2048_less(uint2048_t* a, uint2048_t* b);
bool uint2048_less_equal(uint2048_t* a, uint2048_t* b);

bool uint2048_is_not_zero(uint2048_t* a);
bool uint2048_is_odd(uint2048_t* a);

int uint2048_dump(uint2048_t* a);

#endif // _BOOT_LIBBOOT_CRYPTO_UINT2048_H
