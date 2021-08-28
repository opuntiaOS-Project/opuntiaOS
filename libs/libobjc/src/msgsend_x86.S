extern objc_msg_lookup
global objc_msgSend

objc_msgSend:
    call objc_msg_lookup
    jmp [eax]