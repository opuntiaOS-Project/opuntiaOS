/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "WindowManager.h"

static WindowManager* s_the;

WindowManager& WindowManager::the()
{
    return *s_the;
}

WindowManager::WindowManager()
{
    s_the = this;
}