/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "sha256.h"
#include <libboot/log/log.h>
#include <libboot/mem/mem.h>

const static sha256_word_t sha256_consts[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

const static sha256_word_t sha256_state_init[8] = {
    0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
    0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

#define ROTLEFT(a, b) (((a) << (b)) | ((a) >> (32 - (b))))
#define ROTRIGHT(a, b) (((a) >> (b)) | ((a) << (32 - (b))))

#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTRIGHT(x, 2) ^ ROTRIGHT(x, 13) ^ ROTRIGHT(x, 22))
#define EP1(x) (ROTRIGHT(x, 6) ^ ROTRIGHT(x, 11) ^ ROTRIGHT(x, 25))
#define SIG0(x) (ROTRIGHT(x, 7) ^ ROTRIGHT(x, 18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x, 17) ^ ROTRIGHT(x, 19) ^ ((x) >> 10))

static void sha256_transform(sha256_ctx_t* ctx, const void* vdata)
{
    sha256_byte_t* data = (sha256_byte_t*)vdata;
    sha256_word_t m[64];

    size_t i = 0;
    for (sha256_word_t j = 0; i < 16; i++, j += 4) {
        m[i] = (data[j] << 24) | (data[j + 1] << 16) | (data[j + 2] << 8) | (data[j + 3]);
    }

    for (; i < 64; i++) {
        m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];
    }

    sha256_word_t a = ctx->state[0];
    sha256_word_t b = ctx->state[1];
    sha256_word_t c = ctx->state[2];
    sha256_word_t d = ctx->state[3];
    sha256_word_t e = ctx->state[4];
    sha256_word_t f = ctx->state[5];
    sha256_word_t g = ctx->state[6];
    sha256_word_t h = ctx->state[7];

    for (i = 0; i < 64; i++) {
        sha256_word_t t1 = h + EP1(e) + CH(e, f, g) + sha256_consts[i] + m[i];
        sha256_word_t t2 = EP0(a) + MAJ(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;
}

void sha256_init(sha256_ctx_t* ctx)
{
    ctx->bufnxt = 0;
    ctx->bits_count = 0;
    memcpy(ctx->state, sha256_state_init, sizeof(sha256_state_init));
}

void sha256_update(sha256_ctx_t* ctx, const void* vdata, size_t len)
{
    sha256_byte_t* data = (sha256_byte_t*)vdata;

    for (size_t i = 0; i < len; i++) {
        ctx->buf[ctx->bufnxt] = data[i];
        ctx->bufnxt++;
        if (ctx->bufnxt == 64) {
            sha256_transform(ctx, ctx->buf);
            ctx->bits_count += 512;
            ctx->bufnxt = 0;
        }
    }
}

void sha256_hash(sha256_ctx_t* ctx, char* hash)
{
    size_t i = ctx->bufnxt;

    if (ctx->bufnxt < 56) {
        ctx->buf[i++] = 0x80;
    } else {
        ctx->buf[i++] = 0x80;
        while (i < 64) {
            ctx->buf[i++] = 0x00;
        }
        sha256_transform(ctx, ctx->buf);
        i = 0;
    }

    while (i < 56) {
        ctx->buf[i++] = 0x00;
    }

    ctx->bits_count += ctx->bufnxt * 8;
    ctx->buf[56] = ctx->bits_count >> 56;
    ctx->buf[57] = ctx->bits_count >> 48;
    ctx->buf[58] = ctx->bits_count >> 40;
    ctx->buf[59] = ctx->bits_count >> 32;
    ctx->buf[60] = ctx->bits_count >> 24;
    ctx->buf[61] = ctx->bits_count >> 16;
    ctx->buf[62] = ctx->bits_count >> 8;
    ctx->buf[63] = ctx->bits_count >> 0;

    sha256_transform(ctx, ctx->buf);

    // Making hash big endian.
    for (i = 0; i < 4; i++) {
        hash[i] = (ctx->state[0] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 4] = (ctx->state[1] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 8] = (ctx->state[2] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 12] = (ctx->state[3] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 16] = (ctx->state[4] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 20] = (ctx->state[5] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 24] = (ctx->state[6] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 28] = (ctx->state[7] >> (24 - i * 8)) & 0x000000ff;
    }
}