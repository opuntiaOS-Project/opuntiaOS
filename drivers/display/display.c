#include "display.h"

void printChar(char bo, unsigned char color, int col, int row) {
    char* mem = VIDEO_MEMORY;
    unsigned int position = row * MAX_COLS + col;
    mem = 'X';
    mem[position * 2] = bo;
    mem[position * 2 + 1] = color;
}
