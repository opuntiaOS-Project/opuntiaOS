/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __oneOS__DRIVERS__GENERIC__KEYBOARD_H
#define __oneOS__DRIVERS__GENERIC__KEYBOARD_H

#include <types.h>

enum KEYCODE {

    // Alphanumeric keys ////////////////
    KEY_CTRLC = '\003',

    KEY_SPACE = ' ',
    KEY_0 = '0',
    KEY_1 = '1',
    KEY_2 = '2',
    KEY_3 = '3',
    KEY_4 = '4',
    KEY_5 = '5',
    KEY_6 = '6',
    KEY_7 = '7',
    KEY_8 = '8',
    KEY_9 = '9',

    KEY_A = 'a',
    KEY_B = 'b',
    KEY_C = 'c',
    KEY_D = 'd',
    KEY_E = 'e',
    KEY_F = 'f',
    KEY_G = 'g',
    KEY_H = 'h',
    KEY_I = 'i',
    KEY_J = 'j',
    KEY_K = 'k',
    KEY_L = 'l',
    KEY_M = 'm',
    KEY_N = 'n',
    KEY_O = 'o',
    KEY_P = 'p',
    KEY_Q = 'q',
    KEY_R = 'r',
    KEY_S = 's',
    KEY_T = 't',
    KEY_U = 'u',
    KEY_V = 'v',
    KEY_W = 'w',
    KEY_X = 'x',
    KEY_Y = 'y',
    KEY_Z = 'z',

    KEY_RETURN = '\r',
    KEY_ESCAPE = 0x1001,
    KEY_BACKSPACE = '\b',

    // Arrow keys ////////////////////////

    KEY_UP = 0x1100,
    KEY_DOWN = 0x1101,
    KEY_LEFT = 0x1102,
    KEY_RIGHT = 0x1103,

    // Function keys /////////////////////

    KEY_F1 = 0x1201,
    KEY_F2 = 0x1202,
    KEY_F3 = 0x1203,
    KEY_F4 = 0x1204,
    KEY_F5 = 0x1205,
    KEY_F6 = 0x1206,
    KEY_F7 = 0x1207,
    KEY_F8 = 0x1208,
    KEY_F9 = 0x1209,
    KEY_F10 = 0x120a,
    KEY_F11 = 0x120b,
    KEY_F12 = 0x120b,
    KEY_F13 = 0x120c,
    KEY_F14 = 0x120d,
    KEY_F15 = 0x120e,

    KEY_DOT = '.',
    KEY_COMMA = ',',
    KEY_COLON = ':',
    KEY_SEMICOLON = ';',
    KEY_SLASH = '/',
    KEY_BACKSLASH = '\\',
    KEY_PLUS = '+',
    KEY_MINUS = '-',
    KEY_ASTERISK = '*',
    KEY_EXCLAMATION = '!',
    KEY_QUESTION = '?',
    KEY_QUOTEDOUBLE = '\"',
    KEY_QUOTE = '\'',
    KEY_EQUAL = '=',
    KEY_HASH = '#',
    KEY_PERCENT = '%',
    KEY_AMPERSAND = '&',
    KEY_UNDERSCORE = '_',
    KEY_LEFTPARENTHESIS = '(',
    KEY_RIGHTPARENTHESIS = ')',
    KEY_LEFTBRACKET = '[',
    KEY_RIGHTBRACKET = ']',
    KEY_LEFTCURL = '{',
    KEY_RIGHTCURL = '}',
    KEY_DOLLAR = '$',
    KEY_POUND = 0,
    KEY_EURO = '$',
    KEY_LESS = '<',
    KEY_GREATER = '>',
    KEY_BAR = '|',
    KEY_GRAVE = '`',
    KEY_TILDE = '~',
    KEY_AT = '@',
    KEY_CARRET = '^',

    // Numeric keypad //////////////////////

    KEY_KP_0 = '0',
    KEY_KP_1 = '1',
    KEY_KP_2 = '2',
    KEY_KP_3 = '3',
    KEY_KP_4 = '4',
    KEY_KP_5 = '5',
    KEY_KP_6 = '6',
    KEY_KP_7 = '7',
    KEY_KP_8 = '8',
    KEY_KP_9 = '9',
    KEY_KP_PLUS = '+',
    KEY_KP_MINUS = '-',
    KEY_KP_DECIMAL = '.',
    KEY_KP_DIVIDE = '/',
    KEY_KP_ASTERISK = '*',
    KEY_KP_NUMLOCK = 0x300f,
    KEY_KP_ENTER = 0x3010,

    KEY_TAB = 0x4000,
    KEY_CAPSLOCK = 0x4001,

    // Modify keys ////////////////////////////

    KEY_LSHIFT = 0x4002,
    KEY_LCTRL = 0x4003,
    KEY_LALT = 0x4004,
    KEY_LWIN = 0x4005,
    KEY_RSHIFT = 0x4006,
    KEY_RCTRL = 0x4007,
    KEY_RALT = 0x4008,
    KEY_RWIN = 0x4009,

    KEY_INSERT = 0x400a,
    KEY_DELETE = 0x400b,
    KEY_HOME = 0x400c,
    KEY_END = 0x400d,
    KEY_PAGEUP = 0x400e,
    KEY_PAGEDOWN = 0x400f,
    KEY_SCROLLLOCK = 0x4010,
    KEY_PAUSE = 0x4011,

    // Multimedia keys ////////////////////////

    KEY_PREV_TRACK = 0x5001,
    KEY_NEXT_TRACK = 0x5002,
    KEY_MUTE = 0x5003,
    KEY_CALC = 0x5004,
    KEY_PLAY = 0x5005,
    KEY_STOP = 0x5006,
    KEY_VOL_DOWN = 0x5007,
    KEY_VOL_UP = 0x5008,

    KEY_WWW_HOME = 0x500a,

    KEY_UNKNOWN,
    KEY_NUMKEYCODES
};

typedef uint32_t key_t;

/* The keyboard packet should be aligned to 4 bytes */
struct kbd_packet {
    key_t key;
};
typedef struct kbd_packet kbd_packet_t;

int generic_keyboard_create_devfs();
void generic_keyboard_init();
void generic_emit_key_set1(uint32_t scancode);

#endif //__oneOS__DRIVERS__GENERIC__KEYBOARD_H