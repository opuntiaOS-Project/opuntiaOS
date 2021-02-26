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

static bool _generic_keyboard_can_read(dentry_t* dentry, uint32_t start)
{
    return ringbuffer_space_to_read(&gkeyboard_buffer) >= 1;
}

static int _generic_keyboard_read(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len)
{
    uint32_t leno = ringbuffer_space_to_read(&gkeyboard_buffer);
    if (leno > len) {
        leno = len;
    }
    int res = ringbuffer_read(&gkeyboard_buffer, buf, leno);
    return leno;
}

int generic_keyboard_create_devfs()
{
    dentry_t* mp;
    if (vfs_resolve_path("/dev", &mp) < 0) {
        return -1;
    }

    file_ops_t fops = {0};
    fops.can_read = _generic_keyboard_can_read;
    fops.read = _generic_keyboard_read;
    devfs_inode_t* res = devfs_register(mp, MKDEV(11, 0), "kbd", 3, 0, &fops);

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
            /* FIXME: ifdef here to support console mode */
            /* tty_eat_key(key); */
        }
    }

    ringbuffer_write(&gkeyboard_buffer, (uint8_t*)&packet, sizeof(kbd_packet_t));
}

// TODO: Implement with table
static key_t _generic_keyboard_apply_modifiers(key_t key)
{
    if (_gkeyboard_ctrl_enabled) {
        if (key == 'c') {
            key = KEY_CTRLC;
        }
        return key;
    }

    if (_gkeyboard_shift_enabled || _gkeyboard_caps_enabled) {
        if (key >= 'a' && key <= 'z') {
            key -= 32;
        }
    }

    if (_gkeyboard_shift_enabled) {
        switch (key) {
        case KEY_0:
            key = KEY_RIGHTPARENTHESIS;
            break;
        case KEY_1:
            key = KEY_EXCLAMATION;
            break;
        case KEY_2:
            key = KEY_AT;
            break;
        case KEY_3:
            key = KEY_EXCLAMATION;
            break;
        case KEY_4:
            key = KEY_HASH;
            break;
        case KEY_5:
            key = KEY_PERCENT;
            break;
        case KEY_6:
            key = KEY_CARRET;
            break;
        case KEY_7:
            key = KEY_AMPERSAND;
            break;
        case KEY_8:
            key = KEY_ASTERISK;
            break;
        case KEY_9:
            key = KEY_LEFTPARENTHESIS;
            break;
        case KEY_COMMA:
            key = KEY_LESS;
            break;

        case KEY_DOT:
            key = KEY_GREATER;
            break;

        case KEY_SLASH:
            key = KEY_QUESTION;
            break;

        case KEY_SEMICOLON:
            key = KEY_COLON;
            break;

        case KEY_QUOTE:
            key = KEY_QUOTEDOUBLE;
            break;

        case KEY_LEFTBRACKET:
            key = KEY_LEFTCURL;
            break;

        case KEY_RIGHTBRACKET:
            key = KEY_RIGHTCURL;
            break;

        case KEY_GRAVE:
            key = KEY_TILDE;
            break;

        case KEY_MINUS:
            key = KEY_UNDERSCORE;
            break;

        case KEY_EQUAL:
            key = KEY_PLUS;
            break;

        case KEY_BACKSLASH:
            key = KEY_BAR;
            break;
        }
    }
    return key;
}
