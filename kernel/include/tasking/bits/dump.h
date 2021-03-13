/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_TASKING_BITS_DUMP_H
#define _KERNEL_TASKING_BITS_DUMP_H

#include <libkern/types.h>
#include <tasking/proc.h>

typedef int (*dump_saver_t)(const char*);
typedef ssize_t (*sym_resolver_t)(void* symtab, size_t syms_n, uint32_t tp);

struct dump_data {
    proc_t* p;
    uint32_t entry_point;
    void* syms;
    size_t symsn;
    char* strs;
    dump_saver_t writer;
    sym_resolver_t sym_resolver;
};
typedef struct dump_data dump_data_t;

#endif // _KERNEL_TASKING_BITS_DUMP_H