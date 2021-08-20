#ifndef _LIBOBJC_MODULE_H
#define _LIBOBJC_MODULE_H

struct objc_symtab {
    unsigned long sel_ref_cnt;
    struct objc_selector* refs;
    unsigned short cls_def_cnt;
    unsigned short cat_def_cnt;
    void* defs[1];
};

struct objc_module {
    unsigned long version;
    unsigned long size;
    const char* name;
    struct objc_symtab* symtab;
};

#endif // _LIBOBJC_MODULE_H