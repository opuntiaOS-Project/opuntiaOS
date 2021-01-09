/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <utils/kassert.h>

void kpanic(char* err_msg)
{
    log_error("Kpanic occured %s", err_msg);
    system_stop();
}