/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef STAGE2_SECURITY_VALIDATE_H
#define STAGE2_SECURITY_VALIDATE_H

#include <libboot/abi/drivers.h>
#include <libboot/abi/memory.h>
#include <libboot/types.h>

bool validate_file(const char* path, const char* signature_path, drive_desc_t* drive_desc, fs_desc_t* fs_desc);
bool validate_elf(const char* path, drive_desc_t* drive_desc, fs_desc_t* fs_desc);

#endif // STAGE2_SECURITY_VALIDATE_H