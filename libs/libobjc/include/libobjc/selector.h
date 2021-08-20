#ifndef _LIBOBJC_SELECTOR_H
#define _LIBOBJC_SELECTOR_H

#include <stddef.h>

struct objc_selector {
    void* id;
    const char* types;
};
typedef struct objc_selector* SEL;

static inline bool sel_equal(SEL s1, SEL s2)
{
    if (s1 == NULL || s2 == NULL) {
        return s1 == s2;
    }
    return s1->id == s2->id;
}

void selector_init_table();
void selector_add_from_module(struct objc_selector*);

#endif // _LIBOBJC_SELECTOR_H