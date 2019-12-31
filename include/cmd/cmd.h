#ifndef __oneOS__CMD__CMD_H
#define __oneOS__CMD__CMD_H

#include <drivers/display.h>
#include <drivers/keyboard.h>
#include <mem/malloc.h>

#define MAX_CMD_LENGTH 64

typedef struct {
    char *served_cmd;
    void *handler;
} cmd_redirect_desc_t;

void cmd_install();
bool cmd_register(const char *t_cmd, void* t_handler);

#endif //__oneOS__CMD__CMD_H
