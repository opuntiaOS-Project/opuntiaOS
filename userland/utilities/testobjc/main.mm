#include <libobjc/NSObject.h>
#include <stdio.h>

@interface SampleClass : NSObject
- (void)sampleMethod;
- (void)sampleMethod:(int)val;
@end

@implementation SampleClass

- (void)sampleMethod
{
    printf("Hello, Obj-C!");
}

- (void)sampleMethod:(int)val
{
    printf("Hello, Obj-C!");
}

@end

int main()
{
    SampleClass* sampleClass = [[SampleClass alloc] init];
    [sampleClass sampleMethod];
    [sampleClass sampleMethod:5];
    return 0;
}