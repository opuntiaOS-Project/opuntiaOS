#ifndef _LIBOBJC_CLASS_H
#define _LIBOBJC_CLASS_H

#include <libobjc/module.h>

#define DISPATCH_TABLE_NOT_INITIALIZED (void*)0x0

void class_table_init();
void class_add_from_module(struct objc_symtab* symtab);
IMP class_get_implementation(Class cls, SEL sel);

#endif // _LIBOBJC_CLASS_H