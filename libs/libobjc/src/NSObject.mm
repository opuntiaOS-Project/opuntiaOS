#include <libobjc/objc.h>
#include <libobjc/runtime.h>

OBJC_EXPORT IMP objc_msg_lookup(id receiver, SEL op)
{
    return nil_method;
}

OBJC_EXPORT id objc_msgSend(id reciever, SEL selector)
{
    // FIXME
    return reciever;
}

static inline id call_alloc(Class cls, bool checkNil, bool allocWithZone=false)
{
    if (allocWithZone) {
        // TODO: Implement me
        // return ((id(*)(id, SEL, struct _NSZone *))objc_msgSend)(cls, @selector(allocWithZone:), nil);
    }
    return ((id(*)(id, SEL))objc_msgSend)(cls, @selector(alloc));
}

// Called with [Class alloc]
OBJC_EXPORT id objc_alloc(Class cls)
{
    return call_alloc(cls, true, false);
}

// Called with [[Class alloc] init]
OBJC_EXPORT id objc_alloc_init(Class cls)
{
    // TODO: Implement me
    // return [call_alloc(cls, true, false) init];
    return call_alloc(cls, true, false);
}