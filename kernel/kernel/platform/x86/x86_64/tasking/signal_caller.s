[bits 64]

global signal_caller_start
global signal_caller_end

signal_caller_start:
    
    int 0x80
signal_caller_end: