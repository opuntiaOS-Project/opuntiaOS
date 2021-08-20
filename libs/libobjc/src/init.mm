#include <libobjc/objc.h>
#include <libobjc/runtime.h>
#include <libobjc/module.h>
#include <stdio.h>

// The function is called by constructor of each module.
OBJC_EXPORT void __objc_exec_class(struct objc_module* module)
{
    static bool prepared_data_structures = false;
    
    printf("Called __objc_exec_class, starting to init module\n");
    fflush(stdout);

    if (!prepared_data_structures) {
        printf("    Prepearing ENV");
        fflush(stdout);
        selector_init_table();
        prepared_data_structures = true;
    }


    struct objc_symtab* symtab = module->symtab;
    struct objc_selector* selectors = symtab->refs;

    if (selectors) {
        selector_add_from_module(selectors);
    }

    while (1) { }

}

OBJC_EXPORT Class objc_lookup_class(const char *name)
{
    return (Class)NULL;
}
