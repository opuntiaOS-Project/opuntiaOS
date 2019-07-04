#include "display.h"
#include "../port/port.c"

void print_char(char bo, unsigned char color, int col, int row) {
    char* mem = VIDEO_MEMORY;
    unsigned int position = row * MAX_COLS + col;
    mem[position * 2 + 1] = bo;
    mem[position * 2 + 2] = color;
}

void printString(const char* bo[], unsigned char color, int col, int row) {
    char* mem = VIDEO_MEMORY;
    unsigned int position = row * MAX_COLS + col;
    // TODO print string
}

void clean_screen() {
    char* mem = VIDEO_MEMORY;
    for (int i = 0; i < MAX_COLS; i++){
        for (int j = 0; j < MAX_ROWS; j++){
            print_char(' ', WHITE_ON_BLACK, i, j);
        }
    }
    move_cursor(0, 0);
}

void move_cursor(int col, int row) {
    int offset = row * MAX_COLS + col;
    port_byte_out(REG_SCREEN_CTRL, 14);
    port_byte_out(REG_SCREEN_DATA, (unsigned char)(offset >> 8));
    port_byte_out(REG_SCREEN_CTRL, 15);
    port_byte_out(REG_SCREEN_DATA, (unsigned char)(offset & 0xff));
}