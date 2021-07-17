#include <functional>
#include <iostream>
#include <optional>
#include <map>
#include <stdio.h>
#include <string>
#include <utility>

int main()
{
    std::map<int, int> a;
    a[10] = 20;
    printf("Result: %d", a[10]);
    printf("Result: %d", a[0]);
    printf("Result: %d", a[1123]);
    printf("Result: %d", a.size());
    a.erase(10);
    printf("Result: %d", a.size());
    printf("Result: %d", a[10]);
    printf("Result: %d", a.size());
}