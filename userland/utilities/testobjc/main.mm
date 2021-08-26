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

// - (void)sampleMethod:(int)val
// {
//     printf("Hello, Obj-C! %d", val);
// }

@end

int main()
{
    [SampleClass sampleMethod];
    return 0;
}