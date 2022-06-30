/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

.extern objc_msg_lookup
.global objc_msgSend

objc_msgSend:
    sub sp, sp, #0x50
    stp x0, x1, [sp]
    stp x2, x3, [sp, #0x10]
    stp x4, x5, [sp, #0x20]
    stp x6, x7, [sp, #0x30]
    stp x29, x30, [sp, #0x40]
    bl objc_msg_lookup
    mov x9, x0 // imp pointer
    ldp x0, x1, [sp]
    ldp x2, x3, [sp, #0x10]
    ldp x4, x5, [sp, #0x20]
    ldp x6, x7, [sp, #0x30]
    ldp x29, x30, [sp, #0x40]
    add sp, sp, #0x50
    br x9
