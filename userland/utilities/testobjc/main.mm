#include <libobjc/NSObject.h>
#include <libobjc/helpers.h>
#include <stdio.h>

@interface SampleClass : NSObject
+ (void)sampleMethod;
@end

@implementation SampleClass

+ (void)sampleMethod
{
    printf("Calling static: Hello, from Obj-C!");
}

- (void)sampleMethod
{
    printf("Calling method: Hello, Obj-C! %d", 55);
}

@end

int main()
{
    id objectAlloc = [[SampleClass alloc] init];
    [objectAlloc sampleMethod];
    [SampleClass sampleMethod];
    return 0;
}