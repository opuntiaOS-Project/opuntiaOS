#ifndef __oneOS__X86__TASKING__SIGNAL_H
#define __oneOS__X86__TASKING__SIGNAL_H

#include <types.h>
#include <tasking/tasking.h>

/* TODO: Add more */
enum SIGNAL_ACTION {
    SIGNAL_ACTION_TERMINATE,
    SIGNAL_ACTION_STOP,
    SIGNAL_ACTION_CONTINUE,
};

#endif // __oneOS__X86__TASKING__SIGNAL_H