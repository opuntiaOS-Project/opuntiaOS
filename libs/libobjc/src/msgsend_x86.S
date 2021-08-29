 ;
 ; Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 ;  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 ;
 ; Use of this source code is governed by a BSD-style license that can be
 ; found in the LICENSE file.
 ;

extern objc_msg_lookup
global objc_msgSend

objc_msgSend:
    call objc_msg_lookup
    jmp [eax]