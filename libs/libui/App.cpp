/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "App.h"

namespace UI {

static App* s_the = 0;

App& App::the()
{
    return *s_the;
}

App::App()
    : m_event_loop()
    , m_server_connection(socket(PF_LOCAL, 0, 0))
{
    s_the = this;
}

int App::run()
{
    m_event_loop.run();
    return 0;
}

}