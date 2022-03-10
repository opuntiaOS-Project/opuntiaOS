/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _BOOT_LIBBOOT_CRYPTO_SHA256_H
#define _BOOT_LIBBOOT_CRYPTO_SHA256_H

#include <libboot/mem/mem.h>
#include <libboot/types.h>

typedef uint8_t sha256_byte_t;
typedef uint32_t sha256_word_t;

struct sha256_ctx {
    uint64_t bits_count;
    sha256_word_t state[8];
    sha256_word_t bufnxt;
    sha256_byte_t buf[64];
};
typedef struct sha256_ctx sha256_ctx_t;

void sha256_init(sha256_ctx_t* ctx);
void sha256_update(sha256_ctx_t* ctx, const void* data, size_t len);
void sha256_hash(sha256_ctx_t* ctx, char* hash);

#endif // #define _BOOT_LIBBOOT_CRYPTO_SHA256_H