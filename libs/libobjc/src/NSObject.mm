#include <libobjc/NSObject.h>
#include <libobjc/class.h>
#include <libobjc/memory.h>
#include <libobjc/objc.h>
#include <libobjc/runtime.h>

OBJC_EXPORT IMP objc_msg_lookup(id receiver, SEL sel)
{
    IMP impl = class_get_implementation(receiver->get_isa(), sel);
    return impl;
}

static inline id call_alloc(Class cls, bool checkNil, bool allocWithZone = false)
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

@implementation NSObject

+ (id)init
{
    return (id)self;
}

- (id)init
{
    // Init root classes
    return (id)self;
}

+ (id)alloc
{
    return alloc_instance(self);
}

+ (id)new
{
    return [call_alloc(self, false) init];
}

- (Class)class
{
    return object_getClass(self);
}

@end