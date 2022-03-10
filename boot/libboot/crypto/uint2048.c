/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "uint2048.h"
#include <libboot/log/log.h>
#include <libboot/mem/mem.h>

#define EOVERFLOW 75

int uint2048_init(uint2048_t* d, uint32_t n)
{
    int i = 0;
    d->bucket[i++] = n;
    while (i < N_UINT2048) {
        d->bucket[i++] = 0;
    }
    return 0;
}

int uint2048_init_bytes(uint2048_t* d, const char* f, size_t n)
{
    if (n > sizeof(uint2048_t)) {
        return -1;
    }
    memset(d->bucket, 0, sizeof(uint2048_t));
    memcpy(d->bucket, f, n);
    return 0;
}

static inline uint32_t char_to_u32_safe_convert(char x)
{
    return x > 0 ? x : 256 + x;
}

int uint2048_init_bytes_be(uint2048_t* d, const char* f, size_t n)
{
    if (n > sizeof(uint2048_t)) {
        return -1;
    }

    size_t i = 0;
    uint32_t bytes4 = 0;
    size_t dd = (n / 4) * 4;
    size_t dr = n - dd;
    for (int fi = n - 4; fi >= 0; fi -= 4) {
        bytes4 = 0;
        bytes4 |= char_to_u32_safe_convert(f[fi]) << 24;
        bytes4 |= char_to_u32_safe_convert(f[fi + 1]) << 16;
        bytes4 |= char_to_u32_safe_convert(f[fi + 2]) << 8;
        bytes4 |= char_to_u32_safe_convert(f[fi + 3]) << 0;
        d->bucket[i++] = bytes4;
    }

    bytes4 = 0;
    for (int remi = dr - 1; remi >= 0; remi--) {
        bytes4 |= char_to_u32_safe_convert(f[dd + remi]) << (remi * 8);
    }
    d->bucket[i++] = bytes4;

    while (i < N_UINT2048)
        d->bucket[i++] = 0;

    return 0;
}

int uint2048_copy(uint2048_t* dest, uint2048_t* src)
{
    memcpy(dest, src, sizeof(uint2048_bucket_t) * N_UINT2048);
    return 0;
}

int uint2048_add(uint2048_t* a, uint2048_t* b, uint2048_t* c)
{
    uint64_t carry = 0;
    uint64_t sum = 0;
    for (int i = 0; i < N_UINT2048; i++) {
        uint64_t ab = a->bucket[i];
        uint64_t bb = b->bucket[i];
        sum = ab + bb + carry;
        if (sum >= BASE_UINT2048U) {
            carry = 1;
            sum -= BASE_UINT2048U;
        } else {
            carry = 0;
        }
        c->bucket[i] = (uint2048_bucket_t)sum;
    }

    return carry ? -EOVERFLOW : 0;
}

int uint2048_sub(uint2048_t* a, uint2048_t* b, uint2048_t* c)
{
    uint32_t carry = 0;
    uint64_t sum = 0;
    for (int i = 0; i < N_UINT2048; i++) {
        uint64_t ab = a->bucket[i];
        uint64_t bb = b->bucket[i];
        sum = carry + bb;
        if (ab >= sum) {
            ab = ab - (sum);
            carry = 0;
        } else {
            ab = ab + BASE_UINT2048U - (sum);
            carry = 1;
        }
        c->bucket[i] = (uint2048_bucket_t)ab;
    }

    if (carry) {
        uint2048_init(c, 0);
    }
    return carry ? -EOVERFLOW : 0;
}

int uint2048_shl(uint2048_t* a, int n)
{
    int i;
    for (i = N_UINT2048 - 1; i >= n; i--) {
        a->bucket[i] = a->bucket[i - n];
    }

    while (i >= 0) {
        a->bucket[i--] = 0;
    }

    return 0;
}

int uint2048_shr(uint2048_t* a, int n)
{
    int i;
    for (i = 0; i < N_UINT2048 - n; i++) {
        a->bucket[i] = a->bucket[i + n];
    }
    while (i < N_UINT2048) {
        a->bucket[i++] = 0;
    }

    return 0;
}

int uint2048_mult_by_digit(uint2048_t* a, uint2048_t* b, uint2048_bucket_t un)
{
    uint64_t carry = 0;
    uint64_t tmp;
    uint64_t n = (uint64_t)un;
    for (int i = 0; i < N_UINT2048; i++) {
        uint64_t ab = a->bucket[i];
        tmp = n * ab + carry;
        if (tmp >= BASE_UINT2048U) {
            carry = tmp / BASE_UINT2048U;
            tmp %= BASE_UINT2048U;
        } else {
            carry = 0;
        }

        b->bucket[i] = (uint2048_bucket_t)tmp;
    }

    return carry ? -EOVERFLOW : 0;
}

int uint2048_mult(uint2048_t* a, uint2048_t* b, uint2048_t* c)
{
    uint2048_t p;

    uint2048_init(c, 0);
    for (int i = 0; i < N_UINT2048; i++) {
        uint2048_mult_by_digit(b, &p, a->bucket[i]);
        uint2048_shl(&p, i);
        uint2048_add(c, &p, c);
    }

    return 0;
}

int uint2048_div(uint2048_t* a, uint2048_t* b, uint2048_t* dividend, uint2048_t* reminder)
{
    uint2048_t tmp;

    if (dividend) {
        uint2048_init(dividend, 0);
    }
    uint2048_init(reminder, 0);
    for (int i = N_UINT2048 - 1; i >= 0; i--) {
        uint2048_shl(reminder, 1);
        reminder->bucket[0] = a->bucket[i];
        uint64_t l = 0, r = BASE_UINT2048U;
        while (r - l > 1) {
            uint64_t m = (l + r) / 2;
            uint2048_mult_by_digit(b, &tmp, (uint2048_bucket_t)m);
            if (uint2048_less_equal(&tmp, reminder)) {
                l = m;
            } else {
                r = m;
            }
        }

        if (dividend) {
            dividend->bucket[i] = (uint2048_bucket_t)l;
        }
        uint2048_mult_by_digit(b, &tmp, (uint2048_bucket_t)l);
        uint2048_sub(reminder, &tmp, reminder);
    }

    return 0;
}

int uint2048_pow(uint2048_t* ua, uint2048_t* up, uint2048_t* mod, uint2048_t* ans)
{
    uint2048_t tmp1;
    uint2048_t tmp2;
    uint2048_t const2;
    uint2048_t a;
    uint2048_t p;

    uint2048_copy(&a, ua);
    uint2048_copy(&p, up);

    uint2048_init(ans, 1);
    uint2048_init(&const2, 2);
    while (uint2048_is_not_zero(&p)) {
        if (uint2048_is_odd(&p)) {
            uint2048_mult(ans, &a, &tmp1);
            uint2048_div(&tmp1, mod, NULL, &tmp2);
            uint2048_copy(ans, &tmp2);
        }
        uint2048_mult(&a, &a, &tmp1);
        uint2048_div(&tmp1, mod, NULL, &tmp2);
        uint2048_copy(&a, &tmp2);

        uint2048_div(&p, &const2, &tmp1, &tmp2);
        uint2048_copy(&p, &tmp1);
    }

    return 0;
}

bool uint2048_less(uint2048_t* a, uint2048_t* b)
{
    for (int i = N_UINT2048 - 1; i >= 0; i--) {
        if (a->bucket[i] < b->bucket[i]) {
            return true;
        }
        if (a->bucket[i] > b->bucket[i]) {
            return false;
        }
    }
    return false;
}

bool uint2048_less_equal(uint2048_t* a, uint2048_t* b)
{
    for (int i = N_UINT2048 - 1; i >= 0; i--) {
        if (a->bucket[i] < b->bucket[i]) {
            return true;
        }
        if (a->bucket[i] > b->bucket[i]) {
            return false;
        }
    }
    return true;
}

bool uint2048_equal(uint2048_t* a, uint2048_t* b)
{
    for (int i = 0; i < N_UINT2048; i++) {
        if (a->bucket[i] != b->bucket[i]) {
            return false;
        }
    }
    return true;
}

bool uint2048_is_not_zero(uint2048_t* a)
{
    for (int i = 0; i < N_UINT2048; i++) {
        if (a->bucket[i] != 0) {
            return true;
        }
    }
    return false;
}

bool uint2048_is_odd(uint2048_t* a)
{
    return (a->bucket[0] % 2) == 1;
}

int uint2048_dump(uint2048_t* a)
{
    log("dumping");
    for (int i = N_UINT2048 - 1; i >= 0; i--) {
        log("   %x", a->bucket[i]);
    }
    return 0;
}
