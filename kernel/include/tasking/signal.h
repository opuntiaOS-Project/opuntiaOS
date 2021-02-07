#ifndef __oneOS__X86__TASKING__SIGNAL_H
#define __oneOS__X86__TASKING__SIGNAL_H

#include <types.h>

#define SIGNALS_CNT 32

/* TODO: Add more */
enum SIGNAL_ACTION {
    SIGNAL_ACTION_TERMINATE,
    SIGNAL_ACTION_STOP,
    SIGNAL_ACTION_CONTINUE,
    SIGINT,

};

#endif // __oneOS__X86__TASKING__SIGNAL_H