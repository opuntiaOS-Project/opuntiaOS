#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define true (1)
#define false (0)

char* _cmd_app;
char* _cmd_buffer;
char** _cmd_parsed_buffer;
static int _cmd_buffer_position = 0;
static int _cmd_parsed_buffer_position = 0;
static int running_job = 0;

uint32_t _is_cmd_internal();
void _cmd_buffer_clear();
void _cmd_loop();
void _cmd_loop_start();
void _cmd_loop_end();
void _cmd_input();
void _cmd_processor();
char _cmd_is_ascii(uint32_t key);
char _cmd_cmp_command(const char*);
int16_t _cmd_find_cmd_handler();
uint32_t _cmd_getkeycode();

enum internal_cmd_code {
    CMD_NONE = 0,
    CMD_CD,
};

uint32_t _is_cmd_internal()
{
    if (memcmp(_cmd_buffer, "cd ", 3) == 0) {
        return CMD_CD;
    }
    return CMD_NONE;
}

int _cmd_do_internal(uint32_t code)
{
    if (code == CMD_CD) {
        return chdir(_cmd_parsed_buffer[1]);
    }
    return 0;
}

void _cmd_buffer_clear()
{
    _cmd_buffer_position = 0;
}

void _cmd_loop()
{
    for (;;) {
        _cmd_loop_start();
        _cmd_input();
        _cmd_processor();
        _cmd_loop_end();
    }
}

void _cmd_input()
{
    _cmd_buffer_position = read(STDIN, _cmd_buffer, 256);

    while (_cmd_buffer_position < 0)
        _cmd_buffer_position = read(STDIN, _cmd_buffer, 256);
}

void _cmd_processor()
{
    _cmd_parsed_buffer_position = 0;

    char is_prev_space = true;

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

    /* Remove \n */
    _cmd_buffer[_cmd_buffer_position-1] = '\0';
    _cmd_parsed_buffer[_cmd_parsed_buffer_position] = 0;

    /* We try to launch an app */
    uint32_t cmd = _is_cmd_internal();
    if (cmd == CMD_NONE) {
        uint32_t namelen = strlen(_cmd_parsed_buffer[0]);
        memcpy(_cmd_app + 5, _cmd_buffer, namelen + 1);

        int res = fork();
        if (!res) {
            // We don't pass an app name to args.
            execve(_cmd_app, &_cmd_parsed_buffer[1], 0);
            exit(-1);
        } else {
            running_job = res;
            wait(res);
        }
    } else {
        _cmd_do_internal(cmd);
    }
}

void _cmd_loop_start()
{
    write(1, "> ", 2);
}

void _cmd_loop_end()
{
    _cmd_buffer_clear();
    write(1, "\n", 1);
}

char _cmd_is_ascii(uint32_t key)
{
    return 32 <= key && key <= 126;
}

int inter(int no)
{
    kill(running_job, 9);
    return 0;
}

int main()
{
    sigaction(3, inter);
    ioctl(0, TIOCSPGRP, getpgid(getpid()));
    ioctl(0, TIOCGPGRP, 0);
    _cmd_app = malloc(256);
    _cmd_buffer = malloc(256);
    _cmd_parsed_buffer = malloc(256 * sizeof(char*));
    memcpy(_cmd_app, "/bin/", 5);
    _cmd_loop();

    return 0;
}
