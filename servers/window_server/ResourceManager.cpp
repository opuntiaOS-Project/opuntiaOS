/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "ResourceManager.h"
#include <libg/ImageLoaders/PNGLoader.h>
#include <std/Dbg.h>

static ResourceManager* s_the;

ResourceManager& ResourceManager::the()
{
    return *s_the;
}

ResourceManager::ResourceManager()
{
    s_the = this;
    LG::PNG::PNGLoader loader;
    m_background = loader.load_from_file("/res/background.png");
}