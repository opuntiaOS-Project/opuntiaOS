#ifndef _LIBOBJC_NSOBJECT_H
#define _LIBOBJC_NSOBJECT_H

#include <libobjc/runtime.h>

@interface NSObject
{
    Class isa;
}

- (Class) class;

@end

#endif // _LIBOBJC_NSOBJECT_H