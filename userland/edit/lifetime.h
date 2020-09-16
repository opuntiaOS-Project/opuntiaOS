#ifndef __EDIT__LT__
#define __EDIT__LT__

#include "viewer.h"
#ifdef oneOS
#include <libsystem/termios.h>
#else
#include <termios.h>
#endif

extern struct termios orig_term;

void enable_raw_mode();
void restore_termios();
void exit_app();
void crash_app();

#endif