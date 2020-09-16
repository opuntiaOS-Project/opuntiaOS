#ifndef __EDIT__MODE__
#define __EDIT__MODE__

#include "viewer.h"
#ifdef oneOS
#include <libsystem/types.h>
#else
#include <stdint.h>
#endif

struct mode_disc {
    void (*enter_mode)();
    void (*leave_mode)();
    void (*accept_key)(char key);
};
typedef struct mode_disc mode_disc_t;

enum APP_MODE {
    VIEWING = 0,
    MENU,
    EDITING,
};

extern uint32_t mode;
extern mode_disc_t mode_disc;

#endif