#ifndef __oneLO__DRIVERS__DISAPLAY_H
#define __oneLO__DRIVERS__DISAPLAY_H

#include "port.h"
#include "../types.h"

#define VIDEO_MEMORY (char*)(0xb8000)
#define VIDEO_ADDRESS (char*)(0xb8000)
#define MAX_ROWS 25
#define MAX_COLS 80

#define WHITE_ON_BLACK 0x0f
#define BLACK_ON_WHITE 0xf0
#define RED_ON_BLACK 0x04

#define REG_SCREEN_CTRL 0x3d4
#define REG_SCREEN_DATA 0x3d5

void print_char(char symbol, unsigned char color, int col, int row);
void print_string(const char* string, unsigned char color, int col, int row);
void clean_screen();
void scroll(unsigned char lines);
char col_in_field(int col);
char row_in_field(int row);
void set_cursor_offset(unsigned short offset);
int get_cursor_offet();
int get_offset(int col, int row);

void printf(const char* string);
void printh(uint32_t hex);
void printd(uint32_t dec);

#endif // __oneLO__DRIVERS__DISAPLAY_H