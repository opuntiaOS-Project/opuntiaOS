#include <libobjc/class.h>
#include <libobjc/memory.h>
#include <libobjc/module.h>
#include <libobjc/objc.h>
#include <libobjc/runtime.h>
#include <libobjc/selector.h>
#include <string.h>

struct class_node {
    const char* name;
    int length;
    Class cls;
};

// FIXME: Allocate it dynamically.
static class_node class_tabel_storage[512];
static int class_table_next_free = 0;

void class_table_init()
{
}

Class class_table_find(const char* name)
{
    int len = strlen(name);
    for (int i = 0; i < class_table_next_free; i++) {
        if (len == class_tabel_storage[i].length) {
            if (strcmp(name, class_tabel_storage[i].name)) {
                return class_tabel_storage[i].cls;
            }
        }
    }
    return Nil;
}

static void class_table_add(const char* name, Class cls)
{
    int place = class_table_next_free++;
    class_tabel_storage[place].name = name;
    class_tabel_storage[place].length = strlen(name);
    class_tabel_storage[place].cls = cls;
}

bool class_add(Class cls)
{
    Class fnd = class_table_find(cls->name);
    if (fnd) {
        return false;
    } else {
        static int next_class_num = 1;
        cls->set_number(next_class_num);
        cls->get_isa()->set_number(next_class_num);
        class_table_add(cls->name, cls);
        next_class_num++;
        return true;
    }
}

static inline void class_disp_table_preinit(Class cls)
{
    cls->disp_table = DISPATCH_TABLE_NOT_INITIALIZED;
}

static Method class_lookup_method_in_list(struct objc_method_list* objc_method_list, SEL sel)
{
    if (!selector_is_valid(sel) || !objc_method_list) {
        return (Method)NULL;
    }

    while (objc_method_list) {
        for (int i = 0; i < objc_method_list->method_count; i++) {
            SEL method_name = objc_method_list->method_list[i].method_name;
            if (method_name && method_name->id == sel->id) {
                return &objc_method_list->method_list[i];
            }
        }
        objc_method_list = objc_method_list->method_next;
    }

    return (Method)NULL;
}

static Method class_lookup_method_in_hierarchy(Class cls, SEL sel)
{
    if (!selector_is_valid(sel)) {
        return (Method)NULL;
    }

    Method method;
    for (Class cli = cls; cli; cli = cli->superclass) {
        method = class_lookup_method_in_list(cli->methods, sel);
        if (method) {
            return method;
        }
    }

    return (Method)NULL;
}

// Add instance methods only to root class.
static void class_root_add_instance_methods(Class cls)
{
    int max_methods_allocated = 8;
    struct objc_method_list* new_list = (struct objc_method_list*)objc_malloc(sizeof(struct objc_method_list) + sizeof(struct objc_method[max_methods_allocated]));
    struct objc_method_list* objc_method_list = cls->methods;

    new_list->method_count = 0;

    while (objc_method_list) {
        for (int i = 0; i < objc_method_list->method_count; i++) {
            SEL method_name = objc_method_list->method_list[i].method_name;
            if (method_name) {
                // The instance method isn't a class method yet, so add it.
                if (!class_lookup_method_in_list(cls->get_isa()->methods, method_name)) {
                    new_list->method_list[new_list->method_count++] = objc_method_list->method_list[i];
                    if (new_list->method_count == max_methods_allocated) {
                        max_methods_allocated += 8;
                        new_list = (struct objc_method_list*)objc_realloc(new_list, sizeof(struct objc_method_list) + sizeof(struct objc_method[max_methods_allocated]));
                    }
                }
            }
        }
        objc_method_list = objc_method_list->method_next;
    }

    if (new_list->method_count) {
        new_list->method_next = cls->get_isa()->methods;
        cls->get_isa()->methods = new_list;
    } else {
        objc_free(new_list);
    }

    // TODO: Update dispatch table.
}

bool class_init(Class cls)
{
    if (class_add(cls)) {
        selector_add_from_class(cls);
        selector_add_from_class(cls->get_isa());

        class_disp_table_preinit(cls);
        class_disp_table_preinit(cls->get_isa());

        // TODO: Init methods and dispatch tables.
        if (cls->is_root()) {
            class_root_add_instance_methods(cls);
        }

        return true;
    }

    return false;
}

Class objc_getClass(const char* name)
{
    Class cls = class_table_find(name);
    // TODO: If not found call callbacks.
    return cls;
}

void class_add_from_module(struct objc_symtab* symtab)
{
    for (int i = 0; i < symtab->cls_def_cnt; i++) {
        Class cls = (Class)symtab->defs[i];
        const char* superclass = (char*)cls->superclass;

        OBJC_DEBUGPRINT("Installing classes (%d of %d): %s\n", i + 1, symtab->cls_def_cnt, cls->name);

        // Fix clang flags
        if (cls->is_class()) {
            cls->set_info(CLS_CLASS);
        } else {
            cls->set_info(CLS_META);
        }

        if (class_init(cls)) {
        }
    }
}

OBJC_EXPORT Class objc_lookup_class(const char* name)
{
    return objc_getClass(name);
}