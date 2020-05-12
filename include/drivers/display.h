#ifndef __oneOS__DRIVERS__DISAPLAY_H
#define __oneOS__DRIVERS__DISAPLAY_H

#include <x86/port.h>
#include <types.h>
#include <utils.h>
#include <utils/mem.h>
#include <global.h>

#define VIDEO_MEMORY (char*)(0xb8000 + BIOS_SETTING_BASE)
#define VIDEO_ADDRESS (char*)(0xb8000 + BIOS_SETTING_BASE)
#define MAX_ROWS 25
#define MAX_COLS 80

#define WHITE_ON_BLACK 0x0f
#define BLACK_ON_WHITE 0xf0
#define RED_ON_BLACK 0x04

#define REG_SCREEN_CTRL 0x3d4
#define REG_SCREEN_DATA 0x3d5

#define TRUE 1
#define FALSE 0

typedef unsigned int u_int;

void print_char(char symbol, unsigned char color, int col, int row);
void ext_print_char(char symbol, unsigned char color, int col, int row, int minus_offset);
void delete_char(unsigned char color, int col, int row, int minus_offset);
void print_string(const char* string, unsigned char color, int col, int row);
void print_hex(uint32_t hex, unsigned char color, int col, int row);
void kprintf(const char* string, ...);
void kprinth(uint32_t hex);
void print_dec(uint32_t dec, unsigned char color, int col, int row);
void kprintd(uint32_t dec);
void clean_screen();
void scroll(unsigned char lines);
char col_in_field(int col);
char row_in_field(int row);

void set_cursor_offset(unsigned short offset);
int get_cursor_offet();
int get_offset(int col, int row);

#endif
