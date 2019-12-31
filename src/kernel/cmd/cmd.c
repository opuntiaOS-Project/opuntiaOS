#include <cmd/cmd.h>
#include <fs/vfs.h>

// Private

static char* _cmd_buffer = 0;
static char** _cmd_parsed_buffer = 0;
static uint8_t _cmd_buffer_position = 0;
static uint8_t _cmd_parsed_buffer_position = 0;

static cmd_redirect_desc_t _cmd_redirects[MAX_CMD_LENGTH];
static uint8_t _cmd_redirects_registered = 0;

void _cmd_buffer_clear();
void _cmd_loop();
void _cmd_loop_start();
void _cmd_loop_end();
void _cmd_input();
void _cmd_processor();
bool _cmd_is_ascii(uint32_t key);
bool _cmd_cmp_command(const char *);
int16_t _cmd_find_cmd_handler();
uint32_t _cmd_getkeycode();

bool _cmd_cmp_command(const char * data) {
    uint8_t pos = 0;
    while (data[pos] != '\0') {
        if (data[pos] != _cmd_buffer[pos]) {
            return false;
        }
        pos++;
    }
    return true;
}

int16_t _cmd_find_cmd_handler() {
    for (uint8_t i = 0; i < _cmd_redirects_registered; i++) {
        if (_cmd_cmp_command(_cmd_redirects[i].served_cmd)) {
            return i;
        }
    }
    return -1;
}

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

    _cmd_parsed_buffer_position = 0;

    bool is_prev_space = true;

    for (uint8_t i = 0; i < _cmd_buffer_position; i++) {
        if (_cmd_buffer[i] == ' ') {
            if (is_prev_space == false) {
                // stop sign for apps when attributes are sent
                _cmd_buffer[i] = '\0';
            }
            is_prev_space = true;
        } else {
            if (is_prev_space) {
                _cmd_parsed_buffer[_cmd_parsed_buffer_position++] = (char*)(_cmd_buffer+i);
            }
            is_prev_space = false;
        }
    }
    
    _cmd_buffer[_cmd_buffer_position] = '\0';

    int16_t handler = _cmd_find_cmd_handler();
    if (handler == -1) {
        printf("No such command");
    } else {
        void (*func)(uint8_t args_size, void *args[]) = _cmd_redirects[handler].handler;
        func(_cmd_parsed_buffer_position, _cmd_parsed_buffer);
    }

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
    _cmd_parsed_buffer = kmalloc(sizeof(void*) * 256);
    _cmd_loop();
}

bool cmd_register(const char *t_cmd, void* t_handler) {
    uint8_t len = 0;
    while (t_cmd[len] != '\0') len++;

    char *cmd_holder = kmalloc(len+1);
    memcpy(cmd_holder, t_cmd, len+1);

    if (_cmd_redirects_registered == MAX_CMD_LENGTH) {
        return false;
    }

    _cmd_redirects[_cmd_redirects_registered].served_cmd = cmd_holder;
    _cmd_redirects[_cmd_redirects_registered].handler = t_handler;
    _cmd_redirects_registered++;

    return true;
}
