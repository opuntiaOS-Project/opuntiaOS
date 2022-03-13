/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libboot/crypto/sha256.h>
#include <libboot/crypto/signature.h>
#include <libboot/crypto/uint2048.h>
#include <libboot/crypto/validate.h>
#include <libboot/elf/elf_lite.h>
#include <libboot/fs/ext2_lite.h>
#include <libboot/log/log.h>

#define tmp_buf_size (4096)
char tmp_buf[tmp_buf_size];

static int get_elf_signature(elfctx_t* elfctx, void* signature_buffer)
{
    elf_section_header_32_t shstrtab_section_header;
    elf_read_section_header(elfctx, elfctx->header.e_shstrndx, &shstrtab_section_header);
    uintptr_t shstrtab_offset = shstrtab_section_header.sh_offset;

    for (uint32_t i = 0; i < elfctx->header.e_shnum; i++) {
        elf_section_header_32_t section_header;
        elf_read_section_header(elfctx, i, &section_header);

        char tmp_name_buffer[32];
        uintptr_t name_offset_abs = shstrtab_offset + section_header.sh_name;
        elfctx->fs_desc->read_from_inode(elfctx->drive_desc, &elfctx->file_inode, (void*)tmp_name_buffer, name_offset_abs, 32);
        size_t len = strnlen(tmp_name_buffer, 32);
        if (len != sizeof("._signature") - 1) {
            continue;
        }
        if (memcmp(tmp_name_buffer, "._signature", len)) {
            continue;
        }

        elfctx->fs_desc->read_from_inode(elfctx->drive_desc, &elfctx->file_inode, signature_buffer, section_header.sh_offset, 128);
        return 0;
    }

    return -1;
}

static int calc_elf_hash(elfctx_t* elfctx, char* hash)
{
    sha256_ctx_t shactx;
    sha256_init(&shactx);

    for (uint32_t i = 0; i < elfctx->header.e_phnum; i++) {
        elf_program_header_32_t program_header;
        elf_read_program_header(elfctx, i, &program_header);

        if (program_header.p_type != PT_LOAD) {
            continue;
        }

        size_t from = program_header.p_offset;
        size_t rem_to_read = program_header.p_filesz;

        while (rem_to_read) {
            size_t will_read = min(tmp_buf_size, rem_to_read);
            int rd = elfctx->fs_desc->read_from_inode(elfctx->drive_desc, &elfctx->file_inode, (void*)tmp_buf, from, will_read);
            from += will_read;
            sha256_update(&shactx, tmp_buf, will_read);

            rem_to_read -= will_read;
        }
    }

    sha256_hash(&shactx, hash);
    return 0;
}

bool validate_elf(const char* path, drive_desc_t* drive_desc, fs_desc_t* fs_desc)
{
    elfctx_t elfctx;
    char hash[32];

    int err = elf_init_ctx(drive_desc, fs_desc, path, &elfctx);
    if (err) {
        return false;
    }

    err = get_elf_signature(&elfctx, tmp_buf);
    if (err) {
        return false;
    }
    uint2048_t signature;
    uint2048_init_bytes(&signature, tmp_buf, 128);

    err = calc_elf_hash(&elfctx, hash);
    if (err) {
        return false;
    }

    uint2048_t ihash;
    uint2048_init_bytes_be(&ihash, hash, 32);

    uint2048_t public_e;
    uint2048_init_bytes(&public_e, pub_opuntiaos_key_e, pub_opuntiaos_key_e_len);
    uint2048_t public_n;
    uint2048_init_bytes(&public_n, pub_opuntiaos_key_n, pub_opuntiaos_key_n_len);
    uint2048_t signed_ihash;
    uint2048_pow(&signature, &public_e, &public_n, &signed_ihash);

    return uint2048_equal(&signed_ihash, &ihash);
}