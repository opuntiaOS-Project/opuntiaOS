#include "display.h"

void printChar(char bo, unsigned char color, int col, int row) {
    char* mem = VIDEO_MEMORY;
    unsigned int position = row * MAX_COLS + col;
    mem[position * 2 + 1] = bo;
    mem[position * 2 + 2] = color;
}

void printString(char bo, unsigned char color, int col, int row) {
    char* mem = VIDEO_MEMORY;
    unsigned int position = row * MAX_COLS + col;
    mem[position * 2 + 1] = bo;
    mem[position * 2 + 2] = color;
}

void cleanScreen() {
    char* mem = VIDEO_MEMORY;
    
    for (int i = 0; i < MAX_COLS*MAX_ROWS; i+=2) {
        mem[i] = ' ';
        mem[i+1] = BLACK_ON_WHITE;
    }    
}