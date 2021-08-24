#ifndef _LIBOBJC_SELECTOR_H
#define _LIBOBJC_SELECTOR_H

#include <libobjc/v1/decls.h>
#include <stddef.h>

static inline bool sel_equal(SEL s1, SEL s2)
{
    if (s1 == NULL || s2 == NULL) {
        return s1 == s2;
    }
    return s1->id == s2->id;
}

void selector_table_init();
void selector_add_from_module(struct objc_selector*);
void selector_add_from_method_list(struct objc_method_list*);
void selector_add_from_class(Class);
bool selector_is_valid(SEL sel);

#endif // _LIBOBJC_SELECTOR_H