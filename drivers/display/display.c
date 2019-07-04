#include "display.h"
#include "../port/port.c"
#include "../../kernel/utils.c"

// prints

void print_char(char symbol, unsigned char color, int col, int row) {
    char* mem = VIDEO_MEMORY;
    // unsigned short offset_types[2] = {get_cursor_offet(), get_offset(col, row)};
    // unsigned short offset;
    // offset = offset_types[(col_in_field(col) && row_in_field(row))]; 
    // mem[offset * 2 + 1] = symbol;
    // mem[offset * 2 + 2] = color;
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

    if (offset == MAX_COLS * MAX_ROWS) {
        scroll(1); // scrool for one line;
    } else {
        set_cursor_offset(offset);
    }
}

void printString(const char* bo[], unsigned char color, int col, int row) {
    char* mem = VIDEO_MEMORY;
    unsigned int position = row * MAX_COLS + col;
    // TODO print string
}

// clean screen

void clean_screen() {
    char* mem = VIDEO_MEMORY;
    for (int i = 0; i < MAX_COLS; i++){
        for (int j = 0; j < MAX_ROWS; j++){
            int offset = get_offset(i, j);
            mem[2 * offset + 1] = ' ';
            mem[2 * offset + 2] =  BLACK_ON_WHITE;
        }
    }
    set_cursor_offset(0, 0);
}

// scroll

void scroll (unsigned char lines) {
    char* mem = VIDEO_MEMORY;
    for (int i = lines; i < MAX_ROWS; i++){
        for (int j = 0; j < MAX_COLS; j++) {
            int offset = get_offset(j, i);
            int offsetOld = get_offset(j, i - lines);
            if (offset - offsetOld == 80) {
                mem[1] = 'X';
                mem[3] = 'z';
                mem[1] = mem[3];
            }
            //mem[2 * offsetOld + 1] = mem[2 * offset + 1];
            //mem[2 * offsetOld + 2] = mem[2 * offset + 2];
        }
    }
     //set_cursor_offset(MAX_COLS*MAX_ROWS - offset/2);
    for (int i = 0; i < MAX_COLS; i++){
        for (int j = MAX_ROWS - lines; j < MAX_ROWS; j++){
            int offset = get_offset(i, j);
            mem[2 * offset + 1] = ' ';
            mem[2 * offset + 2] =  WHITE_ON_BLACK;
        }
    }
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
    return offset + 1;
}

// helpers

int get_offset(int col, int row) { return row * MAX_COLS + col + 1; }
bool col_in_field(int col) { return (0 <= col && col <= MAX_COLS); }
bool row_in_field(int row) { return (0 <= row && row <= MAX_ROWS); }

int get_offset_row(int offset) { return offset / (2 * MAX_COLS); }
int get_offset_col(int offset) { return (offset - (get_offset_row(offset)*2*MAX_COLS))/2; }