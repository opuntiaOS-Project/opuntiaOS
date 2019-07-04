#include "display.h"
#include "../port/port.c"

void print_char(char symbol, unsigned char color, int col, int row) {
    char* mem = VIDEO_MEMORY;
    unsigned short offset;
    if (col_in_field(col) && row_in_field(row)) {
        offset = get_offset(col, row);
        mem[2 * offset + 1] = symbol;
        mem[2 * offset + 2] = color;
    } else {
        offset = get_cursor_offet();
        mem[offset * 2 + 1] = symbol;
        mem[offset * 2 + 2] = color;
    }
    set_cursor_offset(offset);
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
    set_cursor_offset(0, 0);
}

void set_cursor_offset(unsigned short offset) {
    port_byte_out(REG_SCREEN_CTRL, 14);
    port_byte_out(REG_SCREEN_DATA, (unsigned char)(offset >> 8));
    port_byte_out(REG_SCREEN_CTRL, 15);
    port_byte_out(REG_SCREEN_DATA, (unsigned char)(offset & 0xff));
}

int get_cursor_offet() {
    port_byte_out(REG_SCREEN_CTRL, 14);
    unsigned short offset = port_byte_in(REG_SCREEN_DATA);
    offset <<= 8;
    port_byte_out(REG_SCREEN_CTRL, 15);
    offset += port_byte_in(REG_SCREEN_DATA);
    return offset + 1;
}

int get_offset(int col, int row) {
    return row * MAX_COLS + col + 1;
}

bool col_in_field(int col) { return (0 <= col && col <= MAX_COLS); }
bool row_in_field(int row) { return (0 <= row && row <= MAX_ROWS); }