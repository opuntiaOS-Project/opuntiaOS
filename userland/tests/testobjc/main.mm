#include <libobjc/NSObject.h>
#include <libobjc/helpers.h>
#include <stdio.h>

@interface SampleClass : NSObject {
@public
    int last_val;
}
+ (void)sampleMethod;
- (void)sampleMethod:(int)val;
- (int)get_last;
@end

@implementation SampleClass

+ (void)sampleMethod
{
    printf("Calling static: Hello, from Obj-C!");
}

- (void)sampleMethod:(int)val
{
    last_val = val;
    printf("Calling method: Hello, Obj-C! %d", val);
}

- (int)get_last
{
    return last_val;
}

@end

int main()
{
    id objectAlloc = [[SampleClass alloc] init];
    [objectAlloc sampleMethod:22];
    [SampleClass sampleMethod];
    printf("Last called with %d", [objectAlloc get_last]);
    return 0;
}