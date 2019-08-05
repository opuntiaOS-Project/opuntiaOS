#ifndef __oneOS__DRIVERS__KEYBOARD_H
#define __oneOS__DRIVERS__KEYBOARD_H

void register_keyboard_driver();
void keyboard_handler();
char start_keyboard();
void stop_keyboard();

#endif
