#include <libobjc/memory.h>

id alloc_instance(Class cls)
{
    size_t sz = cls->size();
    id obj = (id)objc_malloc(sz);
    obj->set_isa(cls);
    return obj;
}