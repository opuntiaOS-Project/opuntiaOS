#include <drivers/keyboard.h>

// Private
// Returns key by scancode
uint32_t _kbdriver_get_keycode(uint8_t t_scancode) {
    uint32_t _kbdriver_scancodes[] = {
        KEY_UNKNOWN,	//0
    	KEY_ESCAPE,		//1
    	KEY_1,			//2
    	KEY_2,			//3
    	KEY_3,			//4
    	KEY_4,			//5
    	KEY_5,			//6
    	KEY_6,			//7
    	KEY_7,			//8
    	KEY_8,			//9
    	KEY_9,			//0xa
    	KEY_0,			//0xb
    	KEY_MINUS,		//0xc
    	KEY_EQUAL,		//0xd
    	KEY_BACKSPACE,	//0xe
    	KEY_TAB,		//0xf
    	KEY_Q,			//0x10
    	KEY_W,			//0x11
    	KEY_E,			//0x12
    	KEY_R,			//0x13
    	KEY_T,			//0x14
    	KEY_Y,			//0x15
    	KEY_U,			//0x16
    	KEY_I,			//0x17
    	KEY_O,			//0x18
    	KEY_P,			//0x19
    	KEY_LEFTBRACKET,//0x1a
    	KEY_RIGHTBRACKET,//0x1b
    	KEY_RETURN,		//0x1c
    	KEY_LCTRL,		//0x1d
    	KEY_A,			//0x1e
    	KEY_S,			//0x1f
    	KEY_D,			//0x20
    	KEY_F,			//0x21
    	KEY_G,			//0x22
    	KEY_H,			//0x23
    	KEY_J,			//0x24
    	KEY_K,			//0x25
    	KEY_L,			//0x26
    	KEY_SEMICOLON,	//0x27
    	KEY_QUOTE,		//0x28
    	KEY_GRAVE,		//0x29
    	KEY_LSHIFT,		//0x2a
    	KEY_BACKSLASH,	//0x2b
    	KEY_Z,			//0x2c
    	KEY_X,			//0x2d
    	KEY_C,			//0x2e
    	KEY_V,			//0x2f
    	KEY_B,			//0x30
    	KEY_N,			//0x31
    	KEY_M,			//0x32
    	KEY_COMMA,		//0x33
    	KEY_DOT,		//0x34
    	KEY_SLASH,		//0x35
    	KEY_RSHIFT,		//0x36
    	KEY_KP_ASTERISK,//0x37
    	KEY_RALT,		//0x38
    	KEY_SPACE,		//0x39
    	KEY_CAPSLOCK,	//0x3a
    	KEY_F1,			//0x3b
    	KEY_F2,			//0x3c
    	KEY_F3,			//0x3d
    	KEY_F4,			//0x3e
    	KEY_F5,			//0x3f
    	KEY_F6,			//0x40
    	KEY_F7,			//0x41
    	KEY_F8,			//0x42
    	KEY_F9,			//0x43
    	KEY_F10,		//0x44
    	KEY_KP_NUMLOCK,	//0x45
    	KEY_SCROLLLOCK,	//0x46
    	KEY_HOME,		//0x47
    	KEY_KP_8,		//0x48	//keypad up arrow
    	KEY_PAGEUP,		//0x49
    	KEY_KP_2,		//0x50	//keypad down arrow
    	KEY_KP_3,		//0x51	//keypad page down
    	KEY_KP_0,		//0x52	//keypad insert key
    	KEY_KP_DECIMAL,	//0x53	//keypad delete key
    	KEY_UNKNOWN,	//0x54
    	KEY_UNKNOWN,	//0x55
    	KEY_UNKNOWN,	//0x56
    	KEY_F11,		//0x57
    	KEY_F12			//0x58
    };
    return _kbdriver_scancodes[t_scancode];
}

static bool _kbdriver_shift_enabled = false;
static bool _kbdriver_ctrl_enabled = false;
static bool _kbdriver_alt_enabled = false;
static uint32_t _kbdriver_last_scancode = KEY_UNKNOWN;
driver_desc_t _keyboard_driver_info();

driver_desc_t _keyboard_driver_info() {
    driver_desc_t kbd_desc;
    kbd_desc.type = DRIVER_INPUT_SYSTEMS_DEVICE;
	kbd_desc.auto_start = true;
    kbd_desc.is_device_driver = false;
	kbd_desc.is_device_needed = false;
	kbd_desc.is_driver_needed = false;
    kbd_desc.functions[DRIVER_INPUT_SYSTEMS_ADD_DEVICE] = kbdriver_run;
    kbd_desc.functions[DRIVER_INPUT_SYSTEMS_GET_LAST_KEY] = kbdriver_get_last_key;
    kbd_desc.functions[DRIVER_INPUT_SYSTEMS_DISCARD_LAST_KEY] = kbdriver_discard_last_key;
    kbd_desc.pci_serve_class = 0xff;
    kbd_desc.pci_serve_subclass = 0xff;
    kbd_desc.pci_serve_vendor_id = 0x00;
    kbd_desc.pci_serve_device_id = 0x00;
    return kbd_desc;
}

// Public

void register_keyboard_driver() { // all drivers start with main
    // register_driver(kbdriver_install, kbdriver_stop);
}

bool kbdriver_install() {
    driver_install(_keyboard_driver_info());
    return true;
}

void kbdriver_run() {
    set_irq_handler(IRQ1, keyboard_handler);
}

// keyboard interrupt handler
void keyboard_handler() {
    uint8_t scancode = port_byte_in(0x60);
    // _kbdriver_last_scancode = scancode;
    if (scancode & 0x80) {
		scancode -= 0x80;
		int key = _kbdriver_get_keycode(scancode);
		switch (key) {
			case KEY_LCTRL:
			case KEY_RCTRL:
				_kbdriver_ctrl_enabled = false;
				break;
			case KEY_LSHIFT:
			case KEY_RSHIFT:
				_kbdriver_shift_enabled = false;
				break;
			case KEY_LALT:
			case KEY_RALT:
				_kbdriver_alt_enabled = false;
				break;
		}
	} else {
        int key = _kbdriver_get_keycode(scancode);
        _kbdriver_last_scancode = scancode;
        switch (key) {
            case KEY_LCTRL:
            case KEY_RCTRL:
                _kbdriver_ctrl_enabled = true;
                break;
            case KEY_LSHIFT:
            case KEY_RSHIFT:
                _kbdriver_shift_enabled = true;
                break;
            case KEY_LALT:
            case KEY_RALT:
                _kbdriver_alt_enabled = true;
                break;
        }
    }
}

char kbdriver_key_to_ascii (uint32_t code) {
    uint32_t key = code;
    if (_kbdriver_shift_enabled) {
        if (key >= 'a' && key <= 'z') {
			key -= 32;
        } else {
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

				case KEY_LEFTBRACKET :
					key = KEY_LEFTCURL;
					break;

				case KEY_RIGHTBRACKET :
					key = KEY_RIGHTCURL;
					break;

				case KEY_GRAVE:
					key = KEY_TILDE;
					break;

				case KEY_MINUS:
					key = KEY_UNDERSCORE;
					break;

				case KEY_PLUS:
					key = KEY_EQUAL;
					break;

				case KEY_BACKSLASH:
					key = KEY_BAR;
					break;
			}
        }
    }
    return key;
}

uint32_t kbdriver_get_last_key() {
    if (_kbdriver_last_scancode == KEY_UNKNOWN) {
        return KEY_UNKNOWN;
    } else {
        return _kbdriver_get_keycode(_kbdriver_last_scancode);
    }
}

void kbdriver_discard_last_key() {
	_kbdriver_last_scancode = KEY_UNKNOWN;
}

void print(char l){
    print_string(l, WHITE_ON_BLACK, -1, -1);
}

void print_letter(uint8_t t_scancode) {
    printd(_kbdriver_get_keycode(t_scancode) == KEY_A);
}
