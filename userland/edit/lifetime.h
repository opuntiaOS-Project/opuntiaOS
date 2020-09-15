#ifndef __EDIT__LT__
#define __EDIT__LT__

#include "viewer.h"
#include <libsystem/termios.h>

extern termios_t orig_term;

void enable_raw_mode();
void restore_termios();
void exit_app();
void crash_app();

#endif