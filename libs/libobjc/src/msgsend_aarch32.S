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
    push    {r0-r3}
    push    {lr}
    bl      objc_msg_lookup
    pop     {lr}
    mov     r12, r0 // imp pointer
    pop     {r0-r3}
    bx      r12
