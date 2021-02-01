#ifndef __oneOS__DRIVERS__KEYBOARD_H
#define __oneOS__DRIVERS__KEYBOARD_H

#include <drivers/driver_manager.h>
#include <drivers/generic/keyboard.h>

void keyboard_handler();
bool kbdriver_install();
void kbdriver_run();

uint32_t kbdriver_get_last_key();
void kbdriver_discard_last_key();

#endif
