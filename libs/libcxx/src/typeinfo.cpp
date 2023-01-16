#include <cstring>
#include <new>
#include <typeinfo>

_LIBCXX_BEGIN_NAMESPACE_STD

type_info::~type_info()
{
}

bool type_info::operator==(const type_info& rhs) const
{
    // GCC has a convention where the typeinfo name for a class which
    // is declared in an anonymous namespace begins with '*'.
    if (name()[0] == '*' || rhs.name()[0] == '*') {
        return false;
    }
    return this == &rhs || strcmp(name(), rhs.name()) == 0;
}

bool type_info::operator!=(const type_info& rhs) const
{
    return !operator==(rhs);
}

bool type_info::before(const type_info& rhs) const
{
    // GCC has a convention where the typeinfo name for a class which
    // is declared in an anonymous namespace begins with '*'.
    if (name()[0] == '*' && rhs.name()[0] == '*') {
        return name() < rhs.name();
    }
    return strcmp(name(), rhs.name()) < 0;
}

_LIBCXX_END_NAMESPACE_STD
