#ifndef _LIBOBJC_V1_DECLS_H
#define _LIBOBJC_V1_DECLS_H

#include <stdint.h>

struct objc_class;
struct objc_object;
struct objc_selector;
struct objc_method;

typedef struct objc_class* Class;
typedef struct objc_object* id;
typedef struct objc_selector* SEL;
typedef struct objc_method* Method;
typedef void (*IMP)(void /* id, SEL, ... */);

#define nil_method (IMP) NULL
#define nil (id) NULL
#define Nil (Class) NULL

struct objc_symtab {
    unsigned long sel_ref_cnt;
    struct objc_selector* refs;
    unsigned short cls_def_cnt;
    unsigned short cat_def_cnt;
    void* defs[1]; // Variable len
};

struct objc_module {
    unsigned long version;
    unsigned long size;
    const char* name;
    struct objc_symtab* symtab;
};

struct objc_object final {
    Class isa;

    inline Class get_isa() { return isa; }
};

struct objc_ivar {
    const char* ivar_name;
    const char* ivar_type;
    int ivar_offset;
};

struct objc_ivar_list {
    int ivar_count;
    struct objc_ivar ivar_list[1]; // Variable len
};

struct objc_method {
    SEL method_name;
    const char* method_types;
    IMP method_imp;
};

struct objc_method_description {
    SEL name;
    char* types;
};

struct objc_method_list {
    struct objc_method_list* method_next;
    int method_count;
    struct objc_method method_list[1]; // Variable len
};

struct objc_method_description_list {
    int count;
    struct objc_method_description list[1]; // Variable len
};

struct objc_protocol {
    struct objc_class* class_pointer;
    char* protocol_name;
    struct objc_protocol_list* protocol_list;
    struct objc_method_description_list* instance_methods;
    struct objc_method_description_list* class_methods;
};

struct objc_protocol_list {
    struct objc_protocol_list* next;
    uint32_t count;
    struct objc_protocol* list[1]; // Variable len
};

#define CLS_CLASS 0x1
#define CLS_META 0x2
#define CLS_INITIALIZED 0x4
#define CLS_RESOLVED 0x8 // This means that it has had correct super and sublinks assigned
#define CLS_INCONSTRUCTION 0x10
#define CLS_NUMBER_OFFSET 16 // Long is 32bit long on our target, we use 16bit for number

struct objc_class final {
    Class isa;
    Class superclass;
    const char* name;
    long version;
    unsigned long info;
    long instance_size;
    struct objc_ivar_list* ivars;
    struct objc_method_list* methods;
    void* disp_table;
    struct objc_class* subclass_list;
    struct objc_class* sibling_class;
    struct objc_protocol_list* protocols;
    void* gc_object_type;

    inline Class get_isa() { return isa; }

    inline void set_info(unsigned long mask) { info |= mask; }
    inline void rem_info(unsigned long mask) { info &= (~mask); }
    inline bool has_info(unsigned long mask) { return ((info & mask) == mask); }
    inline unsigned long get_info() { return info; }

    inline bool is_class() { return has_info(CLS_CLASS); }
    inline bool is_meta() { return has_info(CLS_META); }
    inline bool is_root() { return (bool)superclass; }

    inline void set_number(int num) { info = (info & ((1 << CLS_NUMBER_OFFSET) - 1)) | (num << CLS_NUMBER_OFFSET); }
    inline int number() { return get_info() >> CLS_NUMBER_OFFSET; }
};

struct objc_selector {
    void* id;
    const char* types;
};

#endif // _LIBOBJC_V1_DECLS_H