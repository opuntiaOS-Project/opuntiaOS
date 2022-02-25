/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *  + Contributed by bellrise <bellrise.dev@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <drivers/generic/keyboard.h>
#include <drivers/generic/keyboard_mappings/scancode_set1.h>
#include <fs/devfs/devfs.h>
#include <fs/vfs.h>
#include <libkern/libkern.h>

static ringbuffer_t gkeyboard_buffer;
static bool _gkeyboard_has_prefix_e0 = false;
static bool _gkeyboard_shift_enabled = false;
static bool _gkeyboard_ctrl_enabled = false;
static bool _gkeyboard_alt_enabled = false;
static bool _gkeyboard_caps_enabled = false;
static uint32_t _gkeyboard_last_scancode = KEY_UNKNOWN;

static key_t _generic_keyboard_apply_modifiers(key_t key);

/* Simple struct to store the alternative key when a modifier key is presed. */
struct alternative_key {
    key_t old;
    key_t new;
};

static bool _generic_keyboard_can_read(dentry_t* dentry, size_t start)
{
    return ringbuffer_space_to_read(&gkeyboard_buffer) >= 1;
}

static int _generic_keyboard_read(dentry_t* dentry, uint8_t* buf, size_t start, size_t len)
{
    uint32_t read_len;

    read_len = ringbuffer_space_to_read(&gkeyboard_buffer);
    if (read_len > len)
        read_len = len;

    ringbuffer_read(&gkeyboard_buffer, buf, read_len);
    return read_len;
}

int generic_keyboard_create_devfs()
{
    dentry_t* mp;
    if (vfs_resolve_path("/dev", &mp) < 0)
        return -1;

    file_ops_t fops = { 0 };
    fops.can_read = _generic_keyboard_can_read;
    fops.read = _generic_keyboard_read;
    devfs_inode_t* res = devfs_register(mp, MKDEV(11, 0), "kbd", 3, S_IFCHR | 0400, &fops);

    dentry_put(mp);
    return 0;
}

void generic_keyboard_init()
{
    gkeyboard_buffer = ringbuffer_create_std();
}

void generic_emit_key_set1(uint32_t scancode)
{
    if (scancode == 0xE0) {
        _gkeyboard_has_prefix_e0 = true;
        return;
    }

    if (_gkeyboard_has_prefix_e0) {
        scancode |= 0x100;
        _gkeyboard_has_prefix_e0 = false;
    }

    key_t key;
    kbd_packet_t packet;

    /* Add modifiers */
    if (scancode & 0x80) {
        scancode -= 0x80;
        key = generic_keyboard_get_keycode_set1(scancode);
        packet.key = key | (1 << 31);

        switch (key) {
        case KEY_LCTRL:
        case KEY_RCTRL:
            _gkeyboard_ctrl_enabled = false;
            break;
        case KEY_LSHIFT:
        case KEY_RSHIFT:
            _gkeyboard_shift_enabled = false;
            break;
        case KEY_CAPSLOCK:
            _gkeyboard_caps_enabled ^= 1;
            break;
        case KEY_LALT:
        case KEY_RALT:
            _gkeyboard_alt_enabled = false;
            break;
        default:
            key = _generic_keyboard_apply_modifiers(key);
            packet.key = key | (1 << 31);
        }

    } else {
        _gkeyboard_last_scancode = scancode;
        key = generic_keyboard_get_keycode_set1(scancode);
        packet.key = key;
        switch (key) {
        case KEY_LCTRL:
        case KEY_RCTRL:
            _gkeyboard_ctrl_enabled = true;
            break;
        case KEY_LSHIFT:
        case KEY_RSHIFT:
            _gkeyboard_shift_enabled = true;
            break;
        case KEY_CAPSLOCK:
            break;
        case KEY_LALT:
        case KEY_RALT:
            _gkeyboard_alt_enabled = true;
            break;
        default:
            key = _generic_keyboard_apply_modifiers(key);
            packet.key = key;
#ifdef KERNEL_TEXT_MODE
            tty_eat_key(key);
#endif
        }
    }

    ringbuffer_write(&gkeyboard_buffer, (uint8_t*)&packet, sizeof(kbd_packet_t));
}

static key_t _generic_keyboard_apply_modifiers(key_t key)
{
    static const struct alternative_key shift_keys[] = {
        { KEY_GRAVE, KEY_TILDE },
        { KEY_1, KEY_EXCLAMATION },
        { KEY_2, KEY_AT },
        { KEY_3, KEY_HASH },
        { KEY_4, KEY_DOLLAR },
        { KEY_5, KEY_PERCENT },
        { KEY_6, KEY_CARRET },
        { KEY_7, KEY_AMPERSAND },
        { KEY_8, KEY_ASTERISK },
        { KEY_9, KEY_LEFTPARENTHESIS },
        { KEY_0, KEY_RIGHTPARENTHESIS },
        { KEY_MINUS, KEY_UNDERSCORE },
        { KEY_EQUAL, KEY_PLUS },
        { KEY_LEFTBRACKET, KEY_LEFTCURL },
        { KEY_RIGHTBRACKET, KEY_RIGHTCURL },
        { KEY_BACKSLASH, KEY_BAR },
        { KEY_SEMICOLON, KEY_COLON },
        { KEY_QUOTE, KEY_QUOTEDOUBLE },
        { KEY_COMMA, KEY_LESS },
        { KEY_DOT, KEY_GREATER },
        { KEY_SLASH, KEY_QUESTION },
    };

    static const size_t shift_keys_l = sizeof(shift_keys)
        / sizeof(struct alternative_key);

    if (_gkeyboard_ctrl_enabled) {
        if (key == 'c')
            key = KEY_CTRLC;
        return key;
    }

    if (_gkeyboard_shift_enabled || _gkeyboard_caps_enabled) {
        if (key >= 'a' && key <= 'z')
            key -= 32;
    }

    if (!_gkeyboard_shift_enabled)
        return key;

    for (unsigned int i = 0; i < shift_keys_l; i++) {
        if (shift_keys[i].old == key)
            return shift_keys[i].new;
    }

    return key;
}
