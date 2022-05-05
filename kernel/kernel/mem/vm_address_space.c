/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libkern/bits/errno.h>
#include <mem/kmalloc.h>
#include <mem/vm_address_space.h>
#include <tasking/proc.h>

vm_address_space_t* vm_address_space_alloc()
{
    vm_address_space_t* res = kmalloc(sizeof(vm_address_space_t));
    if (!res) {
        return NULL;
    }

    memset(res, 0, sizeof(vm_address_space_t));
    res->count = 1;
    spinlock_init(&res->lock);
    if (dynarr_init(memzone_t, &res->zones) != 0) {
        kfree(res);
        return NULL;
    }

    return res;
}

int vm_address_space_free(vm_address_space_t* old)
{
    if (!old) {
        return -EINVAL;
    }

    if (old == vmm_get_kernel_address_space()) {
        return -EPERM;
    }

    old->count--;
    if (old->count == 0) {
        vmm_free_address_space(old);
        dynarr_clear(&old->zones);
        kfree(old);
    }

    return 0;
}