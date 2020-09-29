/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#ifndef __oneOS__UTILS__KASSERT_H
#define __oneOS__UTILS__KASSERT_H

#include <drivers/display.h>
#include <types.h>
#include <log.h>

#define ASSERT(x)                                                  \
    if (unlikely(!(x))) {                                                    \
        log("kassert at line %d in %s\n", __LINE__, __FILE__); \
        asm volatile("cli\n");                                     \
        asm volatile("hlt\n");                                     \
    }

#define BUG()                    \
    asm volatile("ud2\n"         \
                 "\t.word %c0\n" \
                 "\t.long %c1\n" \
                 :               \
                 : "i"(__LINE__), "i"(__FILE__))

void kpanic(char* msg);

#endif // __oneOS__UTILS__KASSERT_H
