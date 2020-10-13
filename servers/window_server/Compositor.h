/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#pragma once
#include "WSConnection.h"
#include "WSServerDecoder.h"
#include <libipc/ServerConnection.h>
#include <std/Vector.h>
#include <syscalls.h>

class Compositor {
public:
    static Compositor& the();
    Compositor();
    
    void refresh();
private:
};