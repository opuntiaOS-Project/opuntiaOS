#include <cmd/cmd.h>

// Private

char* _cmd_buffer = 0;
uint8_t _cmd_buffer_position = 0;
void _cmd_buffer_clear();
void _cmd_loop();
void _cmd_loop_start();
void _cmd_loop_end();
void _cmd_input();
void _cmd_processor();
bool _cmd_is_ascii(uint32_t key);
uint32_t _cmd_getkeycode();

void _cmd_buffer_clear() {
    _cmd_buffer_position = 0;
}
void _cmd_loop() {
    while (1) {
        _cmd_loop_start();
        _cmd_input();
        _cmd_processor();
        _cmd_loop_end();
    }
}

void _cmd_input() {
    uint32_t keycode = 0;
    while (keycode != KEY_RETURN) {
        keycode = _cmd_getkeycode();
        if (_cmd_is_ascii(keycode) && _cmd_buffer_position < 255) {
            char ascii_symbol = kbdriver_key_to_ascii(keycode);
            print_char(ascii_symbol, WHITE_ON_BLACK, -1, -1);
            _cmd_buffer[_cmd_buffer_position++] = ascii_symbol;
        }
        if (keycode == KEY_BACKSPACE && _cmd_buffer_position > 0) {
            delete_char(WHITE_ON_BLACK, -1, -1, 1);
            _cmd_buffer_position--;
        }
    }
    _cmd_buffer[_cmd_buffer_position] = '\0';
}

void _cmd_processor() {
    printf("\n");
    printf(_cmd_buffer);
}

void _cmd_loop_start() {
    printf("cmd> ");
}

void _cmd_loop_end() {
    _cmd_buffer_clear();
    printf("\n");
}

bool _cmd_is_ascii(uint32_t key) {
    return 32 <= key && key <= 126;
}

uint32_t _cmd_getkeycode() {
    uint32_t key = KEY_UNKNOWN;
    while (key == KEY_UNKNOWN) {
		key = kbdriver_get_last_key();
    }
    kbdriver_discard_last_key();
	return key;
}

void cmd_install() {
    clean_screen();
    _cmd_buffer = kmalloc(256);
    _cmd_loop();
}
