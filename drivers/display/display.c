#include "display.h"
#include "../port/port.c"
#include "../../kernel/utils.c"

// prints

void print_char(char symbol, unsigned char color, int col, int row) {
    char* mem = VIDEO_MEMORY;
    unsigned short offset_types[2] = {get_cursor_offet(), get_offset(col, row)};
    unsigned short offset = offset_types[(col_in_field(col) && row_in_field(row))]; 
    mem[offset * 2] = symbol;
    mem[offset * 2 + 1] = color;
    if (offset + 1 >= MAX_COLS * MAX_ROWS) {
        scroll(1); // scroll for 1 line
    } else {
        set_cursor_offset(offset+1);
    }
}

void print_string(const char* string, unsigned char color, int col, int row) {
    int i = 0;
    while (string[i] != 0) {
        print_char(string[i++], color, col, row);
        col = row = -1;
    }
}

// clean screen

void clean_screen() {
    char* mem = VIDEO_MEMORY;
    for (int i = 0; i < 2 * MAX_COLS * MAX_ROWS; i+=2) {
        mem[i] = ' ';
        mem[i+1] = WHITE_ON_BLACK;
    }
    set_cursor_offset(0);
}

// scroll

void scroll (unsigned char lines) {
    char* mem = VIDEO_MEMORY;
    *mem = 'C';
    unsigned int mem_offset = lines * MAX_COLS;
    unsigned int bytes_2_del = 2*(MAX_COLS*MAX_ROWS-mem_offset);
    char* src = (mem + mem_offset*2);
    memcpy(src, mem, bytes_2_del);

    for (int i = bytes_2_del; i < 2 * MAX_COLS * MAX_ROWS; i+=2) {
        mem[i] = ' ';
        mem[i + 1] = WHITE_ON_BLACK;
    }

    set_cursor_offset(bytes_2_del/2);
}

// cursor

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
    return offset;
}

// helpers

int get_offset(int col, int row) { return row * MAX_COLS + col; }
bool col_in_field(int col) { return (0 <= col && col <= MAX_COLS); }
bool row_in_field(int row) { return (0 <= row && row <= MAX_ROWS); }
