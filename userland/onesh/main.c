#include <libsystem/malloc.h>
#include <libsystem/string.h>
#include <libsystem/syscalls.h>
#include <libsystem/pthread.h>

char* _cmd_app;
char* _cmd_buffer;
char** _cmd_parsed_buffer;
static int _cmd_buffer_position = 0;
static int _cmd_parsed_buffer_position = 0;

void _cmd_buffer_clear();
void _cmd_loop();
void _cmd_loop_start();
void _cmd_loop_end();
void _cmd_input();
void _cmd_processor();
bool _cmd_is_ascii(uint32_t key);
bool _cmd_cmp_command(const char*);
int16_t _cmd_find_cmd_handler();
uint32_t _cmd_getkeycode();

void _cmd_buffer_clear()
{
    _cmd_buffer_position = 0;
}

void _cmd_loop()
{
    while (1) {
        _cmd_loop_start();
        _cmd_input();
        _cmd_processor();
        _cmd_loop_end();
    }
}

void _cmd_input()
{
    _cmd_buffer_position = read(0, _cmd_buffer, 256);
    _cmd_buffer[_cmd_buffer_position] = '\0';
}

void _cmd_processor()
{
    _cmd_parsed_buffer_position = 0;

    bool is_prev_space = true;

    for (int i = 0; i < _cmd_buffer_position; i++) {
        if (_cmd_buffer[i] == ' ') {
            if (is_prev_space == false) {
                /* null terminator when args are sent */
                _cmd_buffer[i] = '\0';
            }
            is_prev_space = true;
        } else {
            if (is_prev_space) {
                _cmd_parsed_buffer[_cmd_parsed_buffer_position++] = (char*)((char*)_cmd_buffer + i);
            }
            is_prev_space = false;
        }
    }

    _cmd_buffer[_cmd_buffer_position - 1] = '\0'; // remove \n
    _cmd_buffer[_cmd_buffer_position] = '\0';
    _cmd_parsed_buffer[_cmd_parsed_buffer_position] = 0;

    uint32_t namelen = strlen(_cmd_parsed_buffer[0]);
    memcpy(_cmd_app+5, _cmd_buffer, namelen + 1);

    int res = fork();
    if (!res) {
        execve(_cmd_app, _cmd_parsed_buffer, 0);
        return -1;
    } else {
        wait(res);
    }
}

void _cmd_loop_start()
{
    write(1, "oneSh: ", 7);
}

void _cmd_loop_end()
{
    _cmd_buffer_clear();
    write(1, "\n", 1);
}

bool _cmd_is_ascii(uint32_t key)
{
    return 32 <= key && key <= 126;
}

int main()
{
    _cmd_app = malloc(256);
    _cmd_buffer = malloc(256);
    _cmd_parsed_buffer = malloc(256 * sizeof(char*));
    memcpy(_cmd_app, "/bin/", 5);
    _cmd_loop();
    return 0;
}