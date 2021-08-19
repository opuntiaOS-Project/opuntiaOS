#include <stdio.h>

@interface SampleClass
- (void)sampleMethod;
@end

@implementation SampleClass

- (void)sampleMethod {
   printf("Hello, Obj-C!");
}

@end

int main() {
   SampleClass *sampleClass = [[SampleClass alloc]init];
   [sampleClass sampleMethod];
   return 0;
}