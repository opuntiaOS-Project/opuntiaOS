/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

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
    old->count--;
    if (old->count == 0) {
        if (old->pdir && old->pdir != vmm_get_kernel_pdir()) {
            vmm_free_address_space(old);
            dynarr_clear(&old->zones);
        }
        kfree(old);
    }

    return 0;
}