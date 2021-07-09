#include <functional>
#include <iostream>
#include <optional>
#include <stdio.h>
#include <string>

// optional can be used as the return type of a factory that may fail
// std::optional<std::string> create(bool b)
// {
//     if (b)
//         return "Godzilla";
//     return {};
// }

// // std::nullopt can be used to create any (empty) std::optional
// auto create2(bool b)
// {
//     return b ? std::optional<std::string> { "Godzilla" } : std::nullopt;
// }

int main()
{
    int a = 0, b = 0;
    std::cerr << "Input: ";
    scanf("%d %d", &a, &b);
    printf("Result: %d", a + b);
}