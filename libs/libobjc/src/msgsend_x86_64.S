 ;
 ; Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 ;  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 ;
 ; Use of this source code is governed by a BSD-style license that can be
 ; found in the LICENSE file.
 ;

extern objc_msg_lookup
global objc_msgSend

objc_msgSend:
    push rdi
    push rsi
    push rdx
    push rcx
    push r8
    push r9
    call objc_msg_lookup
    pop r9
    pop r8
    pop rcx
    pop rdx
    pop rsi
    pop rdi
    jmp [rax]