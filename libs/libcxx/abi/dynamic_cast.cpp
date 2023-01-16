#include "cxxabi.h"
#include "stdio.h"
#include <cstddef>
#include <typeinfo>

extern "C" void* __dynamic_cast(const void* v, const abi::__class_type_info* src, const abi::__class_type_info* dst, std::ptrdiff_t src2dst_offset)
{
    return NULL;
}