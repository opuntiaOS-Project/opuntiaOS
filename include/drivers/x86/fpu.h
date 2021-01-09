/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#ifndef __oneOS__DRIVERS__FPU_H
#define __oneOS__DRIVERS__FPU_H

#include <types.h>

typedef struct {
    uint8_t buffer[512];
} __attribute__((aligned(16))) fpu_state_t;

void fpu_handler();
void fpu_init();
void fpu_reset_state(fpu_state_t* new_fpu_state);

inline void fpu_save(fpu_state_t* fpu_state)
{
    asm volatile("fxsave %0"
                 : "=m"(*fpu_state));
}

inline void fpu_restore(fpu_state_t* fpu_state)
{
    asm volatile("fxrstor %0"
                 :
                 : "m"(*fpu_state));
}

#endif //__oneOS__DRIVERS__FPU_H
