/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "CursorManager.h"
#include <libg/ImageLoaders/PNGLoader.h>
#include <std/Dbg.h>

static CursorManager* s_the;

CursorManager& CursorManager::the()
{
    return *s_the;
}

CursorManager::CursorManager()
{
    s_the = this;
    LG::PNG::PNGLoader loader;
    m_std_cursor = loader.load_from_file("/res/arrow.png");
}